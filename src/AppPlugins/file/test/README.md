# LLT ���з���
1.ִ��LLT׼������
   ����1�� ����Module��FS_Scanner��FS_Backup�ֿ���롣��plugins/fileĿ¼��
       sh build/download_code_for_developer.sh
          
2. ִ��LLT�ı��� 
   ����һ��ʹ��gtest���з�ʽ,��test��ǰĿ¼��ִ��
       cd test
       sh run_gmock_test.sh
   
   �������� ʹ��hdt���з�ʽ,��test��ǰĿ¼��ִ��
       cd test
       sh run_hdt_test.sh

       hdt���е��������ķ���
       sh run_hdt_test.sh --gtest_filter=ApplicationManagerTest*
       ��sh run_hdt_test.sh --gtest_filter=ApplicationManagerTest.ListApplicationResource_native
# DT-FUZZ ���з���
1.ִ��DT-FUZZ׼������
   ����1�� ����Module��FS_Scanner��FS_Backup�ֿ���롣��plugins/fileĿ¼��
       sh build/download_code_for_developer.sh
   ����2�� ����DT-FUZZ���롣��Module/buildĿ¼��
       sh build/download_dtfuzz.sh  
2. ִ��DT-FUZZ�ı��� 
   ����һ��ʹ��gtest���з�ʽ,��test��ǰĿ¼��ִ��
       cd test
       sh run_gmock_test.sh DTFUZZ
   

