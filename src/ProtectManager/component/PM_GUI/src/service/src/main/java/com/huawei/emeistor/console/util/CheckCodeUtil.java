/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package com.huawei.emeistor.console.util;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.awt.Color;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.RenderingHints;
import java.awt.geom.AffineTransform;
import java.awt.image.BufferedImage;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.Objects;
import java.util.Random;

/**
 * 验证码生成工具类
 *
 * @author 马鑫
 * @version RD V100R003C10
 * @since 2019-11-19
 */
public final class CheckCodeUtil {
    private static final Logger LOG = LoggerFactory.getLogger(CheckCodeUtil.class);

    // 移除数字0
    private static final String NUMBERS = "123456789";

    // 移除小写字母oli
    private static final String LOWERS = "abcdefghjkmnpqrstuvwxyz";

    // 移除大写字母O
    private static final String UPPERS = "ABCDEFGHIJKLMNPQRSTUVWXYZ";

    private static final char[][] VALIDATE_CODES =
        new char[][] {NUMBERS.toCharArray(), LOWERS.toCharArray(), UPPERS.toCharArray()};
    private static final int NAGATIVE_ONE = -1;
    private static final int ONE = 1;
    private static final int TWO = 2;
    private static final int THREE = 3;
    private static final int SIX = 6;
    private static final int SEVEN = 7;
    private static final int EIGHT = 8;
    private static final int TEN = 10;
    private static final int TWEELVE = 12;
    private static final int THIRTEEN = 13;
    private static final int SEVENTEEN = 17;
    private static final int TWENTY = 20;
    private static final int TWENTY_THREE = 23;
    private static final int SIXTY = 60;
    private static final int IMGWIDTH = 68;
    private static final int HUNDRED = 100;
    private static final int TWOHANDRED = 200;
    private static final int TWOFOURHANDRED = 240;
    private static final int TWOTWOFIVE = 225;

    /**
     * 随机生成
     */
    private static SecureRandom sRand;

    /**
     * 静态代码块，产生安全随机数
     */
    static {
        try {
            sRand = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            LOG.error("Failed to generate sRand exception:", e);
        }
    }

    /**
     * 构造函数
     */
    private CheckCodeUtil() {
    }

    /**
     * <br>
     *
     * @param scode 传递验证码
     * @return BufferedImage对象
     */
    public static BufferedImage createImage(String scode) {
        // 图象宽度与高度
        int width = IMGWIDTH;
        int height = TWENTY_THREE;

        BufferedImage image = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);

        // 获取图形上下文
        Graphics2D g2 = null;
        Graphics graphics = image.getGraphics();
        if (graphics instanceof Graphics2D) {
            g2 = (Graphics2D) graphics;
        }

        if (g2 == null) {
            return image;
        }

        // 设定背景色
        Objects.requireNonNull(g2).setColor(getRandColor(TWOHANDRED, TWOFOURHANDRED));

        // 对指定的矩形区域填充颜色
        g2.fillRect(0, 0, width, height);

        // 去除线条的锯齿状
        g2.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
        g2.setRenderingHint(RenderingHints.KEY_ALPHA_INTERPOLATION, RenderingHints.VALUE_ALPHA_INTERPOLATION_QUALITY);
        try {
            Random rand = SecureRandom.getInstanceStrong();
            // 8条干扰线
            for (int count = 0; count < SIXTY; count++) {
                g2.setColor(new Color(TWOTWOFIVE - rand.nextInt(HUNDRED + SIXTY),
                    TWOTWOFIVE - rand.nextInt(HUNDRED + SIXTY), TWOTWOFIVE - rand.nextInt(HUNDRED + SIXTY)));
                g2.drawLine(rand.nextInt(width), rand.nextInt(height), rand.nextInt(width), rand.nextInt(height));
            }

            int fontsize;
            int fontstyle;
            int codeLength = scode.length();
            for (int count = 0; count < codeLength; count++) {
                // 设置字体大小和风格
                fontsize = TWENTY + rand.nextInt(THREE);
                fontstyle = rand.nextInt(SIX);
                g2.setFont(new Font("Arial Italic", fontstyle, fontsize));
                // 字符距左边宽度 字符距上边高度
                int charWidth = (width - TWEELVE) / codeLength;
                int charHeight = SEVENTEEN;
                // 图片旋转
                AffineTransform affine = new AffineTransform();
                affine.setToRotation(Math.PI / SEVEN * rand.nextDouble() * (rand.nextBoolean() ? ONE : NAGATIVE_ONE),
                    charWidth * count + TEN, charHeight);
                g2.setTransform(affine);

                String temp = scode.substring(count, count + ONE);
                g2.setColor(new Color(TEN + rand.nextInt(SIXTY), TEN + rand.nextInt(SIXTY), TEN + rand.nextInt(SIXTY)));
                g2.drawString(temp, charWidth * count + SEVEN, charHeight);
            }
        } catch (NoSuchAlgorithmException e) {
            LOG.error("Failed to generate random exception:", e);
        }
        // 图像生效
        g2.dispose();
        return image;
    }

    /**
     * 给定范围获得随机颜色
     *
     * @param fc int
     * @param bc int
     * @return Color
     */
    private static Color getRandColor(int fc, int bc) {
        try {
            SecureRandom random = SecureRandom.getInstanceStrong();
            int tmp = bc;
            if (bc > TWOTWOFIVE) {
                tmp = TWOTWOFIVE;
            }
            int red = fc + random.nextInt(tmp - fc);
            int greed = fc + random.nextInt(tmp - fc);
            int black = fc + random.nextInt(tmp - fc);
            return new Color(red, greed, black);
        } catch (NoSuchAlgorithmException e) {
            LOG.error("Failed to generate random exception:", e);
        }
        return new Color(0, 0, 0);
    }

    /**
     * 字符串
     *
     * @param length 为字符串的长度
     * @return 随机字符串
     */
    public static String runVerifyCode(int length) {
        return new String(rand(length));
    }

    private static char[] rand(int length) {
        int[] bits = new int[VALIDATE_CODES.length];
        int bound = bits.length + 1;
        char[] codes = new char[length];
        bits[0] = 1;
        for (int i = 1; i < VALIDATE_CODES.length; i++) {
            bits[i] = bits[i - 1] << 1;
        }
        for (int i = 0; i < length; i++) {
            generateRandCode(length, bits, bound, codes, i);
        }
        for (int i = 0; i < length; i++) {
            int code = codes[i];
            if (code >= VALIDATE_CODES.length) {
                code = sRand.nextInt(VALIDATE_CODES.length);
            }
            char[] ranges = VALIDATE_CODES[code];
            codes[i] = ranges[sRand.nextInt(ranges.length)];
        }
        return codes;
    }

    private static void generateRandCode(int length, int[] bits, int bound, char[] codes, int index) {
        int count = 0;
        int flags = 0;
        boolean isRun = true;
        while (isRun) {
            int flag = sRand.nextInt(bound);
            if (flag < VALIDATE_CODES.length) {
                int bit = bits[flag];
                if ((flags & bit) != 0 && (length - index <= bits.length - count)) {
                    continue;
                }
                if ((flags & bit) == 0) {
                    count++;
                }

                flags |= bit;
                codes[index] = (char) flag;
                isRun = false;
            } else if (length - index > bits.length - count) {
                codes[index] = (char) VALIDATE_CODES.length;
                isRun = false;
            } else {
                continue;
            }
        }
    }
}
