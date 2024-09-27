package openbackup.access.framework.resource.validator.mock_context;

import java.util.Arrays;

/**
 * 功能描述
 *
 * @author w00616953
 * @since 2021-10-12
 */
public class Product {

    private int productId;
    private String productName;
    private double price;
    private String[] tags;
    private Dimensions dimensions;

    public int getProductId() {
        return productId;
    }

    public void setProductId(int productId) {
        this.productId = productId;
    }

    public String getProductName() {
        return productName;
    }

    public void setProductName(String productName) {
        this.productName = productName;
    }

    public double getPrice() {
        return price;
    }

    public void setPrice(double price) {
        this.price = price;
    }

    public String[] getTags() {
        return tags;
    }

    public void setTags(String[] tags) {
        this.tags = tags;
    }

    public Dimensions getDimensions() {
        return dimensions;
    }

    public void setDimensions(Dimensions dimensions) {
        this.dimensions = dimensions;
    }

    @Override
    public String toString() {
        return "Product{" + "productId=" + productId + ", productName='" + productName + '\'' + ", price=" + price
                + ", tags=" + Arrays.toString(tags) + ", dimensions=" + dimensions + '}';
    }
}
