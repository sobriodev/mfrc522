GOOGLETEST_DIR='googletest'
CMOCK_DIR='cmock'

echo '*** Installing dependencies ***'

# Googletest framework
echo '*** Installing googletest framework ***'
git clone https://github.com/google/googletest $GOOGLETEST_DIR
cd $GOOGLETEST_DIR || exit
cmake .
make
make install
cd ..

# CMock library
echo '*** Installing CMock library ***'
git clone https://github.com/hjagodzinski/C-Mock $CMOCK_DIR
cd $CMOCK_DIR || exit
make install
cd ..

# Cpp-coveralls
echo '*** Installing cpp-coveralls ***'
pip3 install --user cpp-coveralls