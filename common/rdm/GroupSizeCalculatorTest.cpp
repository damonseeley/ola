/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * GroupSizeCalculatorTest.cpp
 * Test fixture for the GroupSizeCalculator and StaticGroupTokenCalculator
 * classes.
 * Copyright (C) 2011 Simon Newton
 */

#include <cppunit/extensions/HelperMacros.h>
#include <vector>

#include "ola/Logging.h"
#include "ola/messaging/Descriptor.h"
#include "common/rdm/GroupSizeCalculator.h"


using ola::messaging::BoolFieldDescriptor;
using ola::messaging::Descriptor;
using ola::messaging::FieldDescriptor;
using ola::messaging::FieldDescriptorGroup;
using ola::messaging::Int16FieldDescriptor;
using ola::messaging::Int32FieldDescriptor;
using ola::messaging::Int8FieldDescriptor;
using ola::messaging::StringFieldDescriptor;
using ola::messaging::UInt16FieldDescriptor;
using ola::messaging::UInt32FieldDescriptor;
using ola::messaging::UInt8FieldDescriptor;
using ola::rdm::GroupSizeCalculator;
using std::vector;


class GroupSizeCalculatorTest: public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(GroupSizeCalculatorTest);
  CPPUNIT_TEST(testSimpleCases);
  CPPUNIT_TEST(testWithFixedGroups);
  CPPUNIT_TEST(testSingleVariableSizedGroup);
  CPPUNIT_TEST(testMultipleVariableSizedGroups);
  CPPUNIT_TEST(testNestedVariableSizedGroups);
  CPPUNIT_TEST_SUITE_END();

  public:
    void testSimpleCases();
    void testWithFixedGroups();
    void testSingleVariableSizedGroup();
    void testMultipleVariableSizedGroups();
    void testNestedVariableSizedGroups();

    void setUp() {
      ola::InitLogging(ola::OLA_LOG_DEBUG, ola::OLA_LOG_STDERR);
    }

  private:
    ola::rdm::GroupSizeCalculator m_calculator;
    ola::rdm::StaticGroupTokenCalculator m_static_calculator;
};


CPPUNIT_TEST_SUITE_REGISTRATION(GroupSizeCalculatorTest);


/**
 * Test that we can determine the token count for simple descriptors.
 */
void GroupSizeCalculatorTest::testSimpleCases() {
  vector<const FieldDescriptor*> fields;
  fields.push_back(new BoolFieldDescriptor("bool1"));
  fields.push_back(new UInt8FieldDescriptor("uint8"));
  fields.push_back(new UInt16FieldDescriptor("uint16"));
  fields.push_back(new UInt32FieldDescriptor("uint32"));
  fields.push_back(new Int8FieldDescriptor("int8"));
  fields.push_back(new Int16FieldDescriptor("int16"));
  fields.push_back(new Int32FieldDescriptor("int32"));
  fields.push_back(new StringFieldDescriptor("string", 0, 32));
  Descriptor descriptor("Test Descriptor", fields);

  unsigned int token_count, group_repeat_count;
  CPPUNIT_ASSERT(
      m_static_calculator.CalculateTokensRequired(&descriptor, &token_count));
  CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(8), token_count);

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::INSUFFICIENT_INPUTS,
      m_calculator.CalculateGroupSize(
        1,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::INSUFFICIENT_INPUTS,
      m_calculator.CalculateGroupSize(
        7,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::NO_VARIABLE_GROUPS,
      m_calculator.CalculateGroupSize(
        8,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::EXTRA_INPUTS,
      m_calculator.CalculateGroupSize(
        9,
        &descriptor,
        &group_repeat_count));
}


/**
 * Check the calculators work with fixed groups.
 */
void GroupSizeCalculatorTest::testWithFixedGroups() {
  vector<const FieldDescriptor*> group_fields, group_fields2;
  group_fields.push_back(new BoolFieldDescriptor("bool"));
  group_fields.push_back(new UInt8FieldDescriptor("uint8"));

  const FieldDescriptorGroup *fixed_group = new FieldDescriptorGroup(
      "", group_fields, 2, 2);

  group_fields2.push_back(new Int16FieldDescriptor("int16"));
  group_fields2.push_back(new UInt16FieldDescriptor("uint16"));
  group_fields2.push_back(new BoolFieldDescriptor("uint16"));

  const FieldDescriptorGroup *fixed_group2 = new FieldDescriptorGroup(
      "", group_fields2, 4, 4);

  unsigned int token_count, group_repeat_count;
  // first check the static calculator
  CPPUNIT_ASSERT(
      m_static_calculator.CalculateTokensRequired(fixed_group, &token_count));
  CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), token_count);

  CPPUNIT_ASSERT(
      m_static_calculator.CalculateTokensRequired(fixed_group2, &token_count));
  CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), token_count);

  // now check the main calculator
  vector<const FieldDescriptor*> fields;
  fields.push_back(fixed_group);
  fields.push_back(fixed_group2);
  Descriptor descriptor("Test Descriptor", fields);

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::INSUFFICIENT_INPUTS,
      m_calculator.CalculateGroupSize(
        4,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::INSUFFICIENT_INPUTS,
      m_calculator.CalculateGroupSize(
        12,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::INSUFFICIENT_INPUTS,
      m_calculator.CalculateGroupSize(
        15,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::NO_VARIABLE_GROUPS,
      m_calculator.CalculateGroupSize(
        16,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::EXTRA_INPUTS,
      m_calculator.CalculateGroupSize(
        17,
        &descriptor,
        &group_repeat_count));
}


/*
 * Test that a single variable-sized group passes
 */
void GroupSizeCalculatorTest::testSingleVariableSizedGroup() {
  vector<const FieldDescriptor*> group_fields, group_fields2;
  group_fields.push_back(new BoolFieldDescriptor("bool"));
  group_fields.push_back(new UInt8FieldDescriptor("uint8"));

  const FieldDescriptorGroup *variable_group = new FieldDescriptorGroup(
      "", group_fields, 0, 2);

  vector<const FieldDescriptor*> fields;
  // add some static fields as well
  fields.push_back(new UInt8FieldDescriptor("uint8"));
  fields.push_back(new UInt16FieldDescriptor("uint16"));
  fields.push_back(variable_group);
  fields.push_back(new UInt32FieldDescriptor("uint32"));
  Descriptor descriptor("Test Descriptor", fields);

  unsigned int group_repeat_count;
  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::INSUFFICIENT_INPUTS,
      m_calculator.CalculateGroupSize(
        0,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::INSUFFICIENT_INPUTS,
      m_calculator.CalculateGroupSize(
        2,
        &descriptor,
        &group_repeat_count));

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::SINGLE_VARIABLE_GROUP,
      m_calculator.CalculateGroupSize(
        3,
        &descriptor,
        &group_repeat_count));
  CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(0), group_repeat_count);

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::SINGLE_VARIABLE_GROUP,
      m_calculator.CalculateGroupSize(
        5,
        &descriptor,
        &group_repeat_count));
  CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(1), group_repeat_count);

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::SINGLE_VARIABLE_GROUP,
      m_calculator.CalculateGroupSize(
        7,
        &descriptor,
        &group_repeat_count));
  CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), group_repeat_count);

  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::MISMATCHED_INPUTS,
      m_calculator.CalculateGroupSize(
        8,
        &descriptor,
        &group_repeat_count));
}


/**
 * Test that multiple variable-sized groups fail
 */
void GroupSizeCalculatorTest::testMultipleVariableSizedGroups() {
  vector<const FieldDescriptor*> group_fields, group_fields2;
  group_fields.push_back(new BoolFieldDescriptor("bool"));
  group_fields.push_back(new UInt8FieldDescriptor("uint8"));

  const FieldDescriptorGroup *variable_group = new FieldDescriptorGroup(
      "", group_fields, 0, 2);

  group_fields2.push_back(new Int16FieldDescriptor("int16"));
  group_fields2.push_back(new UInt16FieldDescriptor("uint16"));
  group_fields2.push_back(new BoolFieldDescriptor("uint16"));

  const FieldDescriptorGroup *variable_group2 = new FieldDescriptorGroup(
      "", group_fields2, 0, 4);

  vector<const FieldDescriptor*> fields;
  fields.push_back(variable_group);
  fields.push_back(variable_group2);
  Descriptor descriptor("Test Descriptor", fields);

  unsigned int token_count, group_repeat_count;
  // first check these the static calculator
  CPPUNIT_ASSERT(
      m_static_calculator.CalculateTokensRequired(variable_group,
                                                  &token_count));
  CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(2), token_count);

  CPPUNIT_ASSERT(
      m_static_calculator.CalculateTokensRequired(variable_group2,
                                                  &token_count));
  CPPUNIT_ASSERT_EQUAL(static_cast<unsigned int>(3), token_count);

  // now check the main calculator.
  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::MULTIPLE_VARIABLE_GROUPS,
      m_calculator.CalculateGroupSize(
        10,
        &descriptor,
        &group_repeat_count));
}


/*
 * Test that a nested, variable sized groups fail
 */
void GroupSizeCalculatorTest::testNestedVariableSizedGroups() {
  vector<const FieldDescriptor*> fields, group_fields, group_fields2;
  group_fields.push_back(new BoolFieldDescriptor("bool"));
  group_fields.push_back(new UInt8FieldDescriptor("uint8"));

  group_fields2.push_back(new Int16FieldDescriptor("int16"));
  group_fields2.push_back(new FieldDescriptorGroup("", group_fields, 0, 2));

  const FieldDescriptorGroup *nested_variable_group = new FieldDescriptorGroup(
      "", group_fields2, 0, 4);

  fields.push_back(new Int16FieldDescriptor("int16"));
  fields.push_back(nested_variable_group);
  Descriptor descriptor("Test Descriptor", fields);

  // first check these the static calculator
  unsigned int token_count, group_repeat_count;
  CPPUNIT_ASSERT(
      !m_static_calculator.CalculateTokensRequired(
        nested_variable_group,
        &token_count));

  // now check the main calculator.
  CPPUNIT_ASSERT_EQUAL(
      GroupSizeCalculator::NESTED_VARIABLE_GROUPS,
      m_calculator.CalculateGroupSize(
        10,
        &descriptor,
        &group_repeat_count));
}
