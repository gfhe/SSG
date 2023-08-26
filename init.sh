sudo apt update
sudo apt install g++ cmake libboost-dev libgoogle-perftools-dev
cd third_party/pybind11 && git submodule update --init --recursive && cd -
mkdir -p build && cd build
cmake .. && make -j