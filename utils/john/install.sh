curl -k https://www.openwall.com/john/k/john-1.9.0-jumbo-1.tar.xz --output john.tar.xz

tar xJvf john.tar.xz

cd john-1.9.0-jumbo-1
cd src
./configure
make -s clean && make -sj4
