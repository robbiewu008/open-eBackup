package openbackup.system.base.sdk.dee;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.dee.model.DownloadFilesRequest;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;

/**
 * dee BaseParse
 *
 * @author jwx701567
 * @since 2022-01-28
 */
@FeignClient(name = "deeBaseParseRest", url = "${protectengine-e-dee-base-parser.url}/v1/internal",
    configuration = CommonFeignConfiguration.class)
public interface DeeBaseParseRest {
    /**
     * 下载副本中的文件
     *
     * @param downloadFilesRequest 下载副本中的文件请求体
     */
    @PostMapping("/flr/action/export")
    void downloadFiles(@RequestBody DownloadFilesRequest downloadFilesRequest);

    /**
     * 关闭副本guest system浏览
     *
     * @param copyId 副本id
     */
    @ExterAttack
    @PutMapping("/browse/guest-system/close/{copyId}")
    void closeCopyGuestSystem(@PathVariable @RequestParam("copyId") String copyId);
}
