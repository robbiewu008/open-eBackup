# HostAgent
# ���뷽ʽ

# һ��cmake ���롾Դ���롿
  ����Agent/buildĿ¼
  1���޸�env.sh�ļ��ֶ� BUILD_CMAKE=ON
  2��ִ��export HOME=Դ����Agent�ϲ�·��
  3�����뻷��������source env.sh
  4��ִ��sh agent_pack_backup.sh

# ����cmake ���롾LLT��
  ����Agent/buildĿ¼
  1���޸�env.sh�ļ��ֶ� BUILD_CMAKE=ON
  2��ִ��export HOME=Դ����Agent�ϲ�·��
  3�����뻷��������source env.sh
  4��ִ��sh agent_make_cmake.sh LLT [��ִ�г�������Ŀ¼Agent/test/stubTest/bin]

# ����makefile���롾Դ���롿  ---  ---  ������ʹ��
  ����Agent/buildĿ¼
  1���޸�env.sh�ļ��ֶ� BUILD_CMAKE=OFF
  2��ִ��export HOME=Դ����Agent�ϲ�·��
  3�����뻷��������source env.sh
  4��ִ��sh agent_pack_backup.sh

# �ġ�makefile���롾LLT��  ---  ������ʹ��
  ����Agent/test/stubTest/buildĿ¼
  1�����뻷��������source env.sh
  2��ִ��sh test_make.sh [��ִ�г�������Ŀ¼Agent/test/stubTest/bin]