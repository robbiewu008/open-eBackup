当前三方开源下载位置如下
 1.thrift相关的开源下载cmc路径，
   x86_64:
   https://cmc-szv.clouddragon.huawei.com/cmcversion/index/componentVersionView?deltaId=4907911068591104&isSelect=Software&url_data=dorado%3ESUB_SYSTEM%3EPlugins%3Ex86_64%3ENasOpenSource
   
   euler-arm:
   https://cmc-szv.clouddragon.huawei.com/cmcversion/index/componentVersionView?deltaId=4907911068591104&isSelect=Software&url_data=dorado%3ESUB_SYSTEM%3EPlugins%3Eeuler-arm%3ENasOpenSource
 
 2.其他公共开源下载的cmc路径
   x86_64:
   https://cmc-szv.clouddragon.huawei.com/cmcversion/index/componentVersionView?deltaId=4902959151948160&isSelect=Software&url_data=dorado%3ESUB_SYSTEM%3E3rd%3Ex86_64
   
   euler-arm:
   https://cmc-szv.clouddragon.huawei.com/cmcversion/index/componentVersionView?deltaId=4902959151948160&isSelect=Software&url_data=dorado%3ESUB_SYSTEM%3E3rd%3Eeuler-arm
   
  待三方开源编译位置统一后，直接修改脚本，引用新开源。
  新三方使用GCC9.3编译，当前使用GCC7.3编译，需要适配。
  
  下载三方脚本：
   build/open_src/download_3rd.sh