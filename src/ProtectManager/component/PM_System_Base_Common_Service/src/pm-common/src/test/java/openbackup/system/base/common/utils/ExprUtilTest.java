/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExprUtil;

import com.fasterxml.jackson.annotation.JsonIgnore;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ExprUtilTest
 *
 * @author l00272247
 * @since 2020-07-09
 */
public class ExprUtilTest {
    /**
     * testEval
     */
    @Test
    public void testEval() {
        Student student = new Student();
        student.setName("student1");
        List<Teacher> teachers = new ArrayList<>();
        Teacher teacher1 = new Teacher();
        teacher1.teacherI = 0;
        teacher1.setName("teacher1");
        Teacher teacher2 = new Teacher();
        teacher2.teacherI = 1;
        teacher2.setName("teacher2");
        teachers.add(teacher1);
        teachers.add(teacher2);
        student.setTeachers(teachers);
        Map<String, Object> data = new HashMap<>();
        data.put("student", student);
        test1(data);
        test2(data);

        test3(data, student, teacher1, teacher2);

        // 获取student数组中元素的i字段
        Assert.assertEquals(Arrays.asList(0, 1), ExprUtil.eval(data, "student.teachers.*.i"));
        Assert.assertEquals(Arrays.asList(student, student), ExprUtil.eval(data, "student.teachers.*.#s"));

        // 获取student数组中元素的teachers属性的Student列表的名称字段，并将获取的结果合并成为同一个列表。
        Assert.assertEquals(Arrays.asList("student1", "student1"), ExprUtil.eval(data, "student.teachers.*.s.*.#name"));
        Assert.assertEquals(Arrays.asList(Collections.singletonList("student1"), Collections.singletonList("student1")),
            ExprUtil.eval(data, "student.teachers.*.s.*.name"));

        Object result1 = ExprUtil.eval(data, "student.teachers");
        Object result2 = ExprUtil.eval(data, "student.teachers.*");
        Assert.assertEquals(result1, result2);

        // 测试元素nullable的场景
        teachers.add(null);
        Object items = ExprUtil.eval(data, "student.teachers.*?.name");
        Assert.assertEquals(Arrays.asList("teacher1", "teacher2", null), items);

        // 测试获取指定索引的场景
        Assert.assertEquals("teacher1", ExprUtil.eval(data, "student.teachers.0.name"));
        Assert.assertNull(ExprUtil.eval(data, "student.teachers.2?.name"));
        test4(data);

        test5(data);

        Object none = ExprUtil.eval(data, "student.parent.name", false);
        Assert.assertNull(none);
    }

    private void test5(Map<String, Object> data) {
        LegoCheckedException e2 = null;
        try {
            Assert.assertNull(ExprUtil.eval(data, "student.teachers.*.name", true));
        } catch (LegoCheckedException legoCheckedException) {
            e2 = legoCheckedException;
        }
        Assert.assertNotNull(e2);
    }

    private void test4(Map<String, Object> data) {
        // 测试元素不允许为null的场景
        LegoCheckedException e1 = null;
        try {
            Assert.assertNull(ExprUtil.eval(data, "student.teachers.2.name", true));
        } catch (LegoCheckedException legoCheckedException) {
            e1 = legoCheckedException;
        }
        Assert.assertNotNull(e1);
    }

    private void test3(Map<String, Object> data, Student student, Teacher teacher1, Teacher teacher2) {
        teacher1.students = Collections.singletonList(student);
        teacher2.students = Collections.singletonList(student);

        // 获取student数组中元素的teachers属性的name字段。
        // 一个Student可以有多个Teacher，data中student为一个Student实例
        Object names = ExprUtil.eval(data, "student.teachers.*.name");
        Assert.assertEquals(Arrays.asList("teacher1", "teacher2"), names);
    }

    private void test2(Map<String, Object> data) {
        // 用法：string()
        Object json = ExprUtil.eval(data, "student.string()");
        Assert.assertTrue(json instanceof String);
        Assert.assertTrue(((String) json).contains("'teacher1'"));
    }

    private void test1(Map<String, Object> data) {
        // 用法：size()
        Object size = ExprUtil.eval(data, "student.teachers.size()");
        Assert.assertEquals(2, size);
    }

    private static class Person {
        String name;

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        @Override
        public String toString() {
            return "Person{" + "name='" + name + '\'' + '}';
        }
    }

    private static class Student extends Person {
        List<Teacher> teachers;

        public List<Teacher> getTeachers() {
            return teachers;
        }

        public void setTeachers(List<Teacher> teachers) {
            this.teachers = teachers;
        }

        @Override
        public String toString() {
            return "Student{" + "name='" + name + '\'' + ", teachers=" + teachers + '}';
        }
    }

    private static class Teacher extends Person {
        int teacherI;

        @JsonIgnore
        List<Student> students;

        /**
         * get teacherI
         *
         * @return teacherI
         */
        public int getI() {
            return teacherI;
        }

        /**
         * set teacherI
         *
         * @param teacherI teacherI
         */
        public void setI(int teacherI) {
            this.teacherI = teacherI;
        }

        /**
         * get students
         *
         * @return students
         */
        public List<Student> getS() {
            return students;
        }

        /**
         * set students
         *
         * @param students students
         */
        public void setS(List<Student> students) {
            this.students = students;
        }

        @Override
        public String toString() {
            return "Teacher{" + "name='" + name + '\'' + ", i=" + teacherI + ", s=" + students + '}';
        }
    }
}
