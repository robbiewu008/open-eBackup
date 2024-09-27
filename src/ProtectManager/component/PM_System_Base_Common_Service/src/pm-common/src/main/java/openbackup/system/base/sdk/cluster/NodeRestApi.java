package openbackup.system.base.sdk.cluster;

import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RequestPart;
import org.springframework.web.multipart.MultipartFile;

import java.net.URI;

/**
 * 给所有节点发消息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-05
 */
public interface NodeRestApi {
    /**
     * 获取浮动ip
     *
     * @param uri uri
     * @return 浮动ip
     */
    @ExterAttack
    @GetMapping("/v1/internal/service/float-ip")
    String getFloatIp(URI uri);

    /**
     * 同步事件转储文件
     *
     * @param uri uri
     * @param multipartFile multipartFile
     * @param filePath filePath
     */
    @ExterAttack
    @PostMapping(value = "/v1/internal/alarms/dumpfile", consumes = MediaType.MULTIPART_FORM_DATA_VALUE)
    void syncAlarmDumpFile(URI uri, @RequestPart("dumpFile") MultipartFile multipartFile,
        @RequestParam("filePath") String filePath);
}
