cd $WORKSPACE/Agent/open_src
OPEN_SRC_FCGI=fcgi*.tar.gz
OPEN_SRC_JSONCPP=jsoncpp*.tar.gz
OPEN_SRC_NGINX=nginx-release-1.15.8
OPEN_SRC_OPENSSL=openssl*.tar.gz
OPEN_SRC_SNMP=snmp++*.tar.gz
OPEN_SRC_SQLITE=sqlite*.zip
OPEN_SRC_TINYXML=tinyxml*.tar.gz
OPEN_SRC_CURL=curl*.tar.gz

OPEN_SRC_FCGI_DIR=fcgi
OPEN_SRC_JSONCPP_DIR=jsoncpp
OPEN_SRC_NGINX_DIR=nginx_tmp
OPEN_SRC_OPENSSL_DIR=openssl
OPEN_SRC_SQLITE_DIR=sqlite
OPEN_SRC_SNMP_DIR=snmp++
OPEN_SRC_TINYXML_DIR=tinyxml
OPEN_SRC_CURL_DIR=curl

#fcgi
if [ ! -d ${OPEN_SRC_FCGI_DIR} ]
then
    gzip -cd ${OPEN_SRC_FCGI} | tar -xvf -
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_FCGI} .tar.gz`
    mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_FCGI_DIR}
fi
#jsoncpp
if [ ! -d ${OPEN_SRC_JSONCPP_DIR} ]
then
    gzip -cd ${OPEN_SRC_JSONCPP} | tar -xvf -
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_JSONCPP} .tar.gz`
    mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_JSONCPP_DIR}
fi
#nginx
if [ ! -d ${OPEN_SRC_NGINX_DIR} ]
then
    #gzip -cd ${OPEN_SRC_NGINX} | tar -xvf -
    #UNZIPED_DIR_NAME=`basename ${OPEN_SRC_NGINX} .tar.gz`
    unzip ${OPEN_SRC_NGINX}.zip
    mv ${OPEN_SRC_NGINX}/ ${OPEN_SRC_NGINX_DIR}      
fi
#openssl
if [ ! -d ${OPEN_SRC_OPENSSL_DIR} ]
then
    gzip -cd ${OPEN_SRC_OPENSSL} | tar -xvf -
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_OPENSSL} .tar.gz`
    mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_OPENSSL_DIR}
    pushd ${OPEN_SRC_OPENSSL_DIR}
    chmod +x config
    ./config
    popd
fi
#snmp
if [ ! -d ${OPEN_SRC_SNMP_DIR} ]
then
    gzip -cd ${OPEN_SRC_SNMP} | tar -xvf -
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SNMP} .tar.gz`
    mv ${UNZIPED_DIR_NAME}/ ${OPEN_SRC_SNMP_DIR}
fi
#sqlite
if [ ! -d ${OPEN_SRC_SQLITE_DIR} ]
then
    unzip ${OPEN_SRC_SQLITE}
    UNZIPED_DIR_NAME=`basename ${OPEN_SRC_SQLITE} .zip`
    mv ${UNZIPED_DIR_NAME} ${OPEN_SRC_SQLITE_DIR}
fi
#tinyxml
if [ ! -d ${OPEN_SRC_TINYXML_DIR} ]
then
    gzip -cd ${OPEN_SRC_TINYXML} | tar -xvf -
	UNZIPED_DIR_NAME=tinyxml2-7.0.1
	mv ${UNZIPED_DIR_NAME}/ ${OPEN_SRC_TINYXML_DIR}
fi
#curl
if [ ! -d ${OPEN_SRC_CURL_DIR} ]
then
    gzip -cd ${OPEN_SRC_CURL} | tar -xvf -
	UNZIPED_DIR_NAME=`basename ${OPEN_SRC_CURL} .tar.gz`
	mv ${UNZIPED_DIR_NAME}/ ${OPEN_SRC_CURL_DIR}
fi
