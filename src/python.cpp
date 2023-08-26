#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include "distance.h"
#include "parameters.h"
#include "index_random.h"
#include "index_ssg.h"
#include "index_graph.h"

#define NUMPY_C_STYLE py::array::c_style | py:array::forcecast

namespace py = pybind11;

using efanna2e::Metric;
using efanna2e::Parameters;
using efanna2e::Index;
using efanna2e::IndexRandom;
using efanna2e::IndexSSG;

using array = py::array_t<float, py::array::c_style | py::array::forcecast>;

void set_seed(int seed) {
    srand(seed);
}

PYBIND11_MODULE(pyssg, m) {
    m.def("set_seed", &set_seed, "Set C++ random seed");
    m.def("load_data", &efanna2e::load_data, "Load data from SIFT-style binary file");
    // m.def("build_efanna", [](std::string graph, array data, size_t num_data, unsigned dim,
    //     unsigned k,unsigned l, unsigned iter, unsigned s, unsigned r){
    //     efanna2e::IndexRandom init_index(dim, num_data);
    //     efanna2e::IndexGraph index(dim, num_data, efanna2e::L2, (efanna2e::Index*)(&init_index));

    //     efanna2e::Parameters paras;
    //     paras.Set<unsigned>("K", k);
    //     paras.Set<unsigned>("L", l);
    //     paras.Set<unsigned>("iter", iter);
    //     paras.Set<unsigned>("S", s);
    //     paras.Set<unsigned>("R", r);

    //     std::cout << "build efanna index..." << std::endl;
    //     index.Build(num_data, data.data(), paras);

    //     std::cout << "saving efanna index..." << std::endl;
    //     index.Save(graph.c_str());

    // }, "Build efanna index from data");

    // Metric
    py::enum_<Metric>(m, "Metric")
        .value("L2", Metric::L2)
        .value("INNER_PRODUCT", Metric::INNER_PRODUCT)
        .value("FAST_L2", Metric::FAST_L2)
        .value("PQ", Metric::PQ);

    // Parameters
    // py::class_<Parameters>(m, "Parameters")
    //     .def("__getitem__", [](const Parameters& params, std::string key) {
    //         try { return params.GetRaw(key); }
    //         catch (const std::invalid_argument&) {
    //             throw py::key_error("Key '" + key + "' does not exist");
    //         }
    //     })
    //     .def("__setitem__", [](const Parameters& params,
    //                            std::string key, unsigned value) {
    //         params.Set<unsigned>(key, value);
    //     })
    //     .def("__setitem__", [](const Parameters& params,
    //                            std::string key, float value) {
    //         params.Set<float>(key, value);
    //     });

    // IndexRandom
    // py::class_<IndexRandom>(m, "IndexRandom")
    //     .def(py::init<size_t, size_t>())
    //     .def("build", &IndexRandom::Build)
    //     .def("search", [](const IndexRandom& index,
    //                       const float* query, const float* x,
    //                       size_t k, unsigned *indices) {
    //         Parameters params;
    //         index.Search(query, x, k, params, indices);
    //     });

    // IndexSSG
    py::class_<IndexSSG>(m, "IndexSSG")
        .def(py::init([](size_t dim, size_t num_data,
                         Metric metric=Metric::FAST_L2) {
            IndexRandom init_index(dim, num_data);
            IndexSSG* index = new IndexSSG(dim, num_data, metric, &init_index);
            return index;
        }), py::arg("dim"), py::arg("num_data"), py::arg("metric") = Metric::FAST_L2)

        /**
         * Build index from data
         * 控制索引构建的参数有：
         * L：参数的值越大，会考虑更多的邻居节点，可以得到更多的启发，但同时也会增加算法的计算复杂度。
         * R：参数的值越大，会捕捉到图中更长的依赖关系，有助于提高准确率，但同时也会增加计算复杂度。L > R
         * A：控制两个边的角度，角度越大，则图裁剪的越厉害，图中的边越少，计算复杂度越低，但同时也会损失一些准确率。
         * 
         * @param graph: path to knn graph file, not ssg graph
         * @param data: data array, 2D numpy array
         * @param num_data: number of data points in data
         * @param l: L parameter for ssg algorithm, controls the quality of the NSG, the larger the better, L > R
         * @param r: R parameter for ssg algorithm, controls the index size of the graph, the best R is related to the intrinsic dimension of the dataset.
         * @param a: A parameter for ssg graph construction, controls the angle between two edges
         * @param n_try: n_try parameter for DFS search algorithm, used in IndexSSG::DFS_expand()
         * 
        */
        .def("build", [](IndexSSG& index, std::string graph, array data, size_t num_data, unsigned dim,
        unsigned l, unsigned r, unsigned a, unsigned n_try){
            float* data_load = efanna2e::data_align(data.mutable_data(), num_data, dim);

            // Construct Parameters object
            Parameters params;
            params.Set<unsigned>("L", l);
            params.Set<unsigned>("R", r);
            params.Set<unsigned>("A", a);
            params.Set<unsigned>("n_try", n_try);
            params.Set<std::string>("nn_graph_path", graph);

            std::cout << "start build graph..." << std::endl;
            // Do Build SSG from previous knn graph
            index.Build(num_data, data_load, params);
            std::cout << "build graph done" << std::endl;
        })

        /* Load SSG graph along with data */
        .def("load", [](IndexSSG& index,
                        std::string graph, array data) {
            index.Load(graph.c_str());
            index.OptimizeGraph(data.data());
        })

        /* Save graph to file */
        .def("save", &IndexSSG::Save)

        /* Do KNN search
            @param query: an 1-D numpy array represents query
            @param k: number of neighbors to search for
            @param l: L parameter for search algorithm

            @return a list contains K neighbors' indices (start from 0)
         */
        .def("search", [](IndexSSG& index, array query, size_t k, unsigned l)
                          -> std::vector<unsigned> {
            // NOTE: This lambda function will convert numpy array to raw pointer

            if (query.ndim() != 1) {
                throw py::value_error("Query should be 1-D array");
            }
            if (query.shape()[0] != index.GetDimension()) {
                throw py::value_error("Dimension mismatch");
            }

            // Construct Parameters object
            Parameters params;
            params.Set<unsigned>("L_search", l);

            // Result vector, will be converted to list by pybind11 automatically
            std::vector<unsigned> indices(k);

            // Do Search
            index.SearchWithOptGraph(query.data(), k, params, indices.data());

            return indices;
        });
}