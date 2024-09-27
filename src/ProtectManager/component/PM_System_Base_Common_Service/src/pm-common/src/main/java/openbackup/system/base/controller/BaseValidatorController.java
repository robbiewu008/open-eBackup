package openbackup.system.base.controller;

import openbackup.system.base.controller.validator.BaseParamValidator;
import openbackup.system.base.util.SpringBeanUtils;

import org.apache.poi.ss.formula.functions.T;
import org.springframework.web.bind.WebDataBinder;
import org.springframework.web.bind.annotation.InitBinder;

import java.util.HashMap;
import java.util.Map;

/**
 * 基本的controller校验器
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/3/5
 **/
public class BaseValidatorController {
    private static final HashMap<Class<?>, BaseParamValidator<T>> VALIDATOR_MAP = new HashMap<>();

    /**
     * 初始化绑定器，次数用户绑定参数的校验器
     *
     * @param binder 框架中的绑定器
     */
    @InitBinder
    public void initBinder(WebDataBinder binder) {
        if (binder.getTarget() == null) {
            return;
        }
        final Class<?> targetClass = binder.getTarget().getClass();
        if (!VALIDATOR_MAP.containsKey(targetClass)) {
            final Map<String, BaseParamValidator> map = SpringBeanUtils.getBeansByClass(BaseParamValidator.class);
            map.forEach(
                    (key, value) -> {
                        if (value.supports(targetClass)) {
                            VALIDATOR_MAP.put(targetClass, value);
                        }
                    });
        }
        if (VALIDATOR_MAP.containsKey(targetClass)) {
            binder.setValidator(VALIDATOR_MAP.get(targetClass));
        }
    }
}
