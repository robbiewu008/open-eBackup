package openbackup.system.base.common.utils.unit;

/**
 * 容量工具类
 *
 * @author cKF17701
 * @version V100R001C00
 * @since 2019-10-25
 */
public class CapUnit {
    private double capacity;

    private String unit;

    /**
     * 构造函数
     *
     * @param capacity capacity
     * @param unit     unit
     */
    public CapUnit(double capacity, String unit) {
        this.capacity = capacity;
        this.unit = unit;
    }

    /**
     * 获取容量
     *
     * @return double
     */
    public double getCapacity() {
        return capacity;
    }

    /**
     * 设置容量
     *
     * @param capacity 容量
     */
    public void setCapacity(double capacity) {
        this.capacity = capacity;
    }

    /**
     * 获得单位
     *
     * @return String unit
     */
    public String getUnit() {
        return unit;
    }

    /**
     * 设置单位
     *
     * @param unit 设置单位
     */
    public void setUnit(String unit) {
        this.unit = unit;
    }
}
