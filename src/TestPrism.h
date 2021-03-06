#pragma once

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestFixture.h>

#include "srcinput/Prism.h"

class TestPrism : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( TestPrism );
	CPPUNIT_TEST( testOStream );
	CPPUNIT_TEST( testHDFSerialize );
	CPPUNIT_TEST( testHDFSerializeNoPerimeter );
	CPPUNIT_TEST( testCopyCtor );
	CPPUNIT_TEST( timeMemDebug );
	CPPUNIT_TEST( PolygonIntersectsSelf );
	CPPUNIT_TEST( TestPointsCtor );
	CPPUNIT_TEST( testOperatorEquals );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();

protected:
	void testOStream(void);
	void testHDFSerialize(void);
	void testHDFSerializeNoPerimeter(void);
	void testCopyCtor(void);
	void timeMemDebug(void);
	void PolygonIntersectsSelf(void);
	void TestPointsCtor(void);
	void testOperatorEquals(void);
};
