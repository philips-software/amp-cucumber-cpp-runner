mkdir /tmp/fmt10.1.1
cd /tmp/fmt10.1.1
git clone --branch 10.1.1 https://github.com/fmtlib/fmt.git
cd fmt
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DFMT_TEST=OFF -DFMT_DOC=OFF
cmake --build build
sudo cmake --install build
