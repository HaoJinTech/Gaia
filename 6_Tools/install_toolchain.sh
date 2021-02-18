sudo rm -rf /usr/src/aarch64-linux-gnu
sudo tar -xvf gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu.tar.xz -C /usr/src/
sudo mv /usr/src/gcc-linaro-7.5.0-2019.12-x86_64_aarch64-linux-gnu /usr/src/aarch64-linux-gnu

#echo "export ARCH_HOME=/usr/src/aarch64-linux-gnu" >> ~/.bashrc
#echo "export PATH=\$PATH:\$ARCH_HOME/bin" >> ~/.bashrc
#source ~/.bashrc

