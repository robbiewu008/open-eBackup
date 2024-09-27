package openbackup.openstack.protection.access.keystone.dto;

import lombok.Data;

import java.util.Arrays;

/**
 * AuthToken请求参数结构体
 *
 * @author x30038064
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-24
 */
@Data
public class AuthTokenRequestDto {
    private AuthDto auth;

    private AuthTokenRequestDto() {
    }

    /**
     * 初始化请求参数实体
     *
     * @param password 密码
     * @param name 用户名
     * @return 请求参数实体
     */
    public static AuthTokenRequestDto initRequestDto(String password, String name) {
        UserDto user = new UserDto();
        user.setName(name);
        user.setPassword(password);

        DomainDto domain = new DomainDto();
        domain.setName("Default");
        user.setDomain(domain);

        PasswordDto pwd = new PasswordDto();
        pwd.setUser(user);

        IdentityDto identity = new IdentityDto();
        identity.setPassword(pwd);
        identity.setMethods(Arrays.asList("password"));

        AuthDto authDto = new AuthDto();
        authDto.setIdentity(identity);

        ScopeDto scope = new ScopeDto();
        DomainDto scopeDomain = new DomainDto();
        scopeDomain.setName("Default");
        scope.setDomain(scopeDomain);

        authDto.setScope(scope);
        AuthTokenRequestDto authTokenRequestDto = new AuthTokenRequestDto();
        authTokenRequestDto.setAuth(authDto);

        return authTokenRequestDto;
    }

    /**
     * 初始化请求参数实体
     *
     * @param password 密码
     * @param name 名称
     * @param domainName 域名称
     * @return 请求参数实体
     */
    public static AuthTokenRequestDto initRequestDto(String password, String name, String domainName) {
        AuthTokenRequestDto authTokenRequestDto = initRequestDto(password, name);
        authTokenRequestDto.getAuth().getIdentity().getPassword().getUser().getDomain().setName(domainName);
        authTokenRequestDto.getAuth().getScope().getDomain().setName(domainName);
        return authTokenRequestDto;
    }
}
