#!/bin/bash

# 临时脚本

# Kafka Zookeeper address
SERVER="infrastructure-zk-kafka:9092"

# Kafka Broker list
BROKER_LIST="0,1,2"

# Kafka Client
CONFIG="/opt/OceanProtect/consumer.properties"
TOPIC_JSON="/opt/third_data/kafka/topics-to-move.json"
REPLICATION_PLAN_JSON="/opt/third_data/kafka/reassignment-plan.json"
LOG_FILE="failed.log"

NEW_PARTITIONS=6
NEW_REPLICATION_FACTOR=3

# Topic list
TOPICS=$(/usr/local/kafka/kafka-2.12-3.5.0/bin/kafka-topics.sh --bootstrap-server $SERVER --list --command-config $CONFIG)
TOTAL_TOPICS=$(echo "$TOPICS" | wc -l)
COMPLETED_TOPICS=0

for TOPIC in $TOPICS; do
  echo "Handle topic: $TOPIC"

  # 1. Change partitions
  CURRENT_PARTITIONS=$(/usr/local/kafka/kafka-2.12-3.5.0/bin/kafka-topics.sh --bootstrap-server $SERVER --describe --topic $TOPIC --command-config $CONFIG | grep "PartitionCount" | awk '{print $6}')

  if [ $NEW_PARTITIONS -gt $CURRENT_PARTITIONS ]; then
    echo "Add partition $TOPIC to $NEW_PARTITIONS"
    /usr/local/kafka/kafka-2.12-3.5.0/bin/kafka-topics.sh --bootstrap-server $SERVER --alter --topic $TOPIC --partitions $NEW_PARTITIONS --command-config $CONFIG
    if [ $? -ne 0 ]; then
      echo "Failed topic: $TOPIC" | tee -a $LOG_FILE
      continue
    fi
  else
    echo "Topic $TOPIC has more or equal partitions than $NEW_PARTITIONS"
  fi

  # 2. Change replications
  echo "Modify topic replication factor $TOPIC to $NEW_REPLICATION_FACTOR"

  # Make up topics-to-move.json
  cat << EOF > $TOPIC_JSON
  {
    "topics": [
      {
        "topic": "$TOPIC"
      }
    ],
    "version": 1
  }
EOF

  # Set up reassignment.json
  cat << EOF > $REPLICATION_PLAN_JSON
  {
    "version": 1,
    "partitions": [
      {
        "topic": "$TOPIC",
        "partition": 0,
        "replicas": [0,1,2]
      },
      {
        "topic": "$TOPIC",
        "partition": 1,
        "replicas": [0,1,2]
      },
      {
        "topic": "$TOPIC",
        "partition": 2,
        "replicas": [0,1,2]
      },
      {
        "topic": "$TOPIC",
        "partition": 3,
        "replicas": [0,1,2]
      },
      {
        "topic": "$TOPIC",
        "partition": 4,
        "replicas": [0,1,2]
      },
      {
        "topic": "$TOPIC",
        "partition": 5,
        "replicas": [0,1,2]
      }
    ]
  }
EOF

  # Execute replications reassign
  /usr/local/kafka/kafka-2.12-3.5.0/bin/kafka-reassign-partitions.sh --bootstrap-server $SERVER --reassignment-json-file $REPLICATION_PLAN_JSON --command-config $CONFIG --execute
  if [ $? -ne 0 ]; then
    echo "Failed topic: $TOPIC" | tee -a $LOG_FILE
    continue
  fi

  # Verify progress
  /usr/local/kafka/kafka-2.12-3.5.0/bin/kafka-reassign-partitions.sh --bootstrap-server $SERVER --reassignment-json-file $REPLICATION_PLAN_JSON --command-config $CONFIG --verify

  echo "Complete partition and replication factor change for $TOPIC "

  COMPLETED_TOPICS=$((COMPLETED_TOPICS + 1))
  echo "Progress: $COMPLETED_TOPICS out of $TOTAL_TOPICS topics completed."
done

echo "Succeed"
exit 0