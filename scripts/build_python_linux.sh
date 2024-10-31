DIR="$( cd "$( dirname "$0" )" && pwd )"
if [ -f "$DIR/../python/bin/python3" ] ; then 
    echo python exist
else 

  cd "$DIR/../external/cpython"
  git fetch
  git checkout 3.10
  # requirement: openssl
  #   sudo apt update
  #   sudo apt install libssl-dev
  #   sudo apt install zlib1g-dev
  #   sudo apt install libffi-dev
  #   sudo apt-get install redis-server
  # chmod +x build_python_linux.sh
  make clean
  LDFLAGS="-L/usr/lib" CPPFLAGS="-I/usr/include/openssl" CFLAGS="-fPIC" CXXFLAGS="-fPIC" ./configure --with-openssl=/usr --prefix="$DIR/../python" --enable-shared  --enable-optimizations --with-ensurepip=install
  make

  make install

  cd "$DIR/../python/bin"

  ./pip3 install --upgrade pip
  ./pip3 install mediapipe
  ./pip3 install pyglm
  ./pip3 install redis
  ./pip3 install async_timeout

fi