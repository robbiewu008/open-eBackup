# **1、安装LCRP工具** #

请参考此文档
[http://3ms.huawei.com/hi/group/3681/wiki_5643386.html?for_statistic_from=creation_group_wiki#top](http://3ms.huawei.com/hi/group/3681/wiki_5643386.html?for_statistic_from=creation_group_wiki#top)

1. 安装JDK
2. 本地仓工具LCRP下载、配置   （只需配置setting.xml即可，后面的dependency关系不用配置）

**注意**	

配置Setting.xml时**localRepository**为临时缓存目录，如/home/hcpdev/cache，不要设置为代码目录或其下面的子目录。 **agentPath**请设置为代码目录，如/home/hcpdev/ebk_base。

配置完成后，需设置环境变量：


    sudo vim /etc/profile
	
在末尾追加：
    
    export LCRP_HOME=/home/hcpdev/.eBackup/toolchains/lcrp/LCRP_HOME（路径为LCRP.zip解压后的路径）
    export JAVA_HOME=$LCRP_HOME/jre1.8.0_131/jre（路径为java安装路径）
    export PATH=$LCRP_HOME/bin:$JAVA_HOME/bin:$PATH
    export CLASSPATH=$JAVA_HOME/lib:$LCRP_HOME/bin

保存后执行命令使其生效：

    source /etc/profile

**执行下面命令看是否配置正确：**

    hcpdev@ctup000102937:~/lcrp.sh
    parameter is not dryrun or d or u or rebuild or uo or do

如出现无执行权限等回显，请自行给lcrp.sh添加x权限。


# 2、安装k3s、helm、docker#
k3s离线安装：[https://docs.rancher.cn/k3s/installation/offline.html#_2-%E5%88%9B%E5%BB%BA%E9%95%9C%E5%83%8F%E4%BB%93%E5%BA%93yaml%E6%96%87%E4%BB%B6](https://docs.rancher.cn/k3s/installation/offline.html#_2-%E5%88%9B%E5%BB%BA%E9%95%9C%E5%83%8F%E4%BB%93%E5%BA%93yaml%E6%96%87%E4%BB%B6)

helm安装：[https://helm.sh/docs/intro/install](https://helm.sh/docs/intro/install/)

dokcer安装：

	yum install docker-engine

# 3、开始编译,可以选择编译基础镜像和三方基础组件#

## 3.1、重新构建基础设施

     root@ctup000102937:~100p/Infrastructure_OM/Infrastructure/script> sh build.sh
    
## 3.3、重新编译基础镜像 ##

如果想从头开始编译基础镜像，则请进入100p/Infrastructure_OM/Infrastructure/script目录，运行

    root@ctup000102937:~100p/Infrastructure_OM/Infrastructure/script> sh build.sh baseImage

## 3.4、重新编译三方基础组件 ##

如果想从头开始编译三方基础组件，则请进入100p/Infrastructure_OM/Infrastructure/script目录，运行

    root@ctup000102937:~100p/Infrastructure_OM/Infrastructure/script> sh build.sh publicBuild
    
## 3.5、下载ICP上预编译好的基础设施组件 ##

如果不想重新编译三方基础组件，可直接进入Infrastructure_OM/build目录，从CMC下载ICP已编译好的二进制，可以节省大量的时间。

    root@ctup000102937:~100p/Infrastructure_OM/Infrastructure/script> sh download_infrastructure_pkg.sh