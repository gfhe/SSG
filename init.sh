sudo apt update
sudo apt install -y g++ cmake libboost-dev libgoogle-perftools-dev
git submodule update --init --recursive
cd third_party/pybind11 && pip install .
mkdir -p build && cd build
cmake .. && make -j