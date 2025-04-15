# 更新kafka的登录用户名和密码
KAFKA_PATH="/usr/local/kafka/kafka-VERSION"
NODE_NAME=${NODE_NAME}
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/kafka"

# 重启kafka之后，会重新从om获取更新的secret加载至conf文件
echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][Restart_KAFKA: Restarting Kafka Server]"
echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][Restart_KAFKA: Restarting Kafka Server]" >>${LOG_PATH}/install_kafka.log
${KAFKA_PATH}/bin/kafka-server-stop.sh
