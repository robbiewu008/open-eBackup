# ����kafka�ĵ�¼�û���������
KAFKA_PATH="/usr/local/kafka/kafka-VERSION"
NODE_NAME=${NODE_NAME}
LOG_PATH="/opt/OceanProtect/logs/${NODE_NAME}/infrastructure/kafka"

# ����kafka֮�󣬻����´�om��ȡ���µ�secret������conf�ļ�
echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][Restart_KAFKA: Restarting Kafka Server]"
echo "$(date +"[%Y-%m-%d %H:%M:%S,%N]")[INFO][Restart_KAFKA: Restarting Kafka Server]" >>${LOG_PATH}/install_kafka.log
${KAFKA_PATH}/bin/kafka-server-stop.sh
