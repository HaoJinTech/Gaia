# Gaia
3rd edition of radio rack control system, based on LINUX, hardware Raspberry Pi 4B, project name "Gaia"

tips:

busybox: https://blog.csdn.net/weixin_44563759/article/details/109840644

kernel: https://blog.csdn.net/qq_36956154/article/details/100105186

git clone git@github.com:HaoJinTech/Gaia

if git clone faild, update git config
git config --global http.postBuffer 524288000
git config --global core.compression -1    

if the file push to git is larger than 100MB
Git Large File Storage(https://git-lfs.github.com/)
sudo apt install git-lfs
git lfs install
git lfs track "filename"
