一、gtest断言
1.1 Boolean断言类型
		致命断言					非致命断言					 结果
	ASSERT_TRUE(condition)		EXPECT_TRUE(condition)      condition 为true
	ASSERT_FALSE(condition)		EXPECT_FALSE(condition) 	condition 为false
1.2 二元值断言类型
		 致命断言					 非致命断言				 判断结果
	ASSERT_EQ(val1, val2)		EXPECT_EQ(val1, val2)		val1 = val2
	ASSERT_NE(val1, val2)		EXPECT_NE(val1, val2)		val1 != val2
	ASSERT_LT(val1, val2)		EXPECT_LT(val1, val2)		val1 < val2
	ASSERT_LE(val1, val2)		EXPECT_LE(val1, val2)		val1 <= val2
	ASSERT_GT(val1, val2)		EXPECT_GT(val1, val2)		val1 > val2
	ASSERT_GE(val1, val2)		EXPECT_GE(val1, val2)		val1 >= val2
1.3 字符串断言类型	
		 致命断言						 非致命断言						结果
	ASSERT_STREQ(val1, val2)		EXPECT_STREQ(val1, val2)		val1 == val2
	ASSERT_STRNE(val1, val2)		EXPECT_STRNE(val1, val2)		val1 != val2
	ASSERT_STRCASEEQ(val1, val2)	EXPECT_STRCASEEQ(val1, val2)	val1 == val2 (忽略大小写)
	ASSERT_STRCASENE(val1, val2)	EXPECT_STRCASENE(val1, val2)	val1 != val2 (忽略大小写)