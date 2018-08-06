/*******************************************************************************
* Copyright 2016 ROBOTIS CO., LTD.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

/* Authors: Darby Lim, Hye-Jong KIM */

#include "../../include/open_manipulator/OMDynamixel.h"

OMDynamixel::OMDynamixel(uint32_t baud_rate)
{
  dxl_info_.baud_rate = baud_rate;
}

OMDynamixel::~OMDynamixel(){};

bool OMDynamixel::init()
{
  uint8_t get_dxl_id[dxl_info_.size];

  dxl_wb_.begin(DEVICE_NAME, dxl_info_.baud_rate);

  if (dxl_wb_.scan(&get_dxl_id[0], &dxl_info_.size, 30))
  {
    for (uint8_t index = 0; index < dxl_info_.size; index++)
    {
      dxl_id_.push_back(get_dxl_id[index]);
      LOG::INFO("ID : " + String(get_dxl_id[index]));
    }
  }
  else
    return false;

  dxl_id_.reserve(dxl_info_.size);
  radian_value_.reserve(dxl_info_.size);
  torque_value_.reserve(dxl_info_.size);

  dxl_wb_.addSyncWrite(GOAL_POSITION);
  if (dxl_wb_.getProtocolVersion() == 2.0)
    dxl_wb_.addSyncRead(PRESENT_POSITION);

  return true;
}

bool OMDynamixel::setMode(uint8_t id, uint8_t mode)
{
  return dxl_wb_.itemWrite(id, OPERATING_MODE, mode);
}

bool OMDynamixel::setPositionControlMode(uint8_t id)
{
  return dxl_wb_.jointMode(id);
}

bool OMDynamixel::setCurrentBasedPositionControlMode(uint8_t id, uint8_t current)
{
  return dxl_wb_.currentMode(id, current);
}

bool OMDynamixel::setMaxPositionLimit(uint8_t id, float radian)
{
  setDisable(id);

  if (dxl_wb_.getProtocolVersion() == 2.0)
    dxl_wb_.itemWrite(id, MAX_POSITION_LIMIT_ADDR, MAX_POSITION_LIMIT_LENGTH, dxl_wb_.convertRadian2Value(id, radian));
  else
    dxl_wb_.itemWrite(id, CW_ANGLE_LIMIT_ADDR, CW_ANGLE_LIMIT_LENGTH, dxl_wb_.convertRadian2Value(id, radian));

  setEnable(id);
}

bool OMDynamixel::setMinPositionLimit(uint8_t id, float radian)
{
  setDisable(id);

  if (dxl_wb_.getProtocolVersion() == 2.0)
    dxl_wb_.itemWrite(id, MIN_POSITION_LIMIT_ADDR, MIN_POSITION_LIMIT_LENGTH, dxl_wb_.convertRadian2Value(id, radian));
  else
    dxl_wb_.itemWrite(id, CCW_ANGLE_LIMIT_ADDR, CCW_ANGLE_LIMIT_LENGTH, dxl_wb_.convertRadian2Value(id, radian));
    
  setEnable(id);
}

void OMDynamixel::addSyncWriteHandler(const char *table_item)
{
  dxl_wb_.addSyncWrite(table_item);
}

void OMDynamixel::addSyncReadHandler(const char *table_item)
{
  dxl_wb_.addSyncRead(table_item);
}

bool OMDynamixel::setEnable(uint8_t id)
{
  return dxl_wb_.itemWrite(id, TORQUE_ENABLE, true);
}

bool OMDynamixel::setDisable(uint8_t id)
{
  return dxl_wb_.itemWrite(id, TORQUE_ENABLE, false);
}

bool OMDynamixel::enableAllDynamixel()
{
  for (uint8_t index = 0; index < dxl_info_.size; index++)
    dxl_wb_.itemWrite(dxl_id_.at(index), TORQUE_ENABLE, true);

  return true;
}

bool OMDynamixel::disableAllDynamixel()
{
  for (uint8_t index = 0; index < dxl_info_.size; index++)
    dxl_wb_.itemWrite(dxl_id_.at(index), TORQUE_ENABLE, false);
  return true;
}

bool OMDynamixel::setAngle(std::vector<float> radian_vector)
{
  int32_t set_position[dxl_info_.size] = {0, };

  for (uint8_t index = 0; index < dxl_info_.size; index++)
    set_position[index] = dxl_wb_.convertRadian2Value(dxl_id_.at(index), radian_vector.at(index));

  return dxl_wb_.syncWrite(GOAL_POSITION, &set_position[0]);
}

bool OMDynamixel::setAngle(uint8_t id, float radian)
{
  int32_t set_position = dxl_wb_.convertRadian2Value(id, radian);

  return dxl_wb_.itemWrite(id, GOAL_POSITION, set_position);
}

uint8_t OMDynamixel::getDynamixelSize()
{
  return dxl_info_.size;
}

std::vector<uint8_t> OMDynamixel::getDynamixelIDs()
{
  return dxl_id_;
}

uint32_t OMDynamixel::getBaudRate()
{
  return dxl_info_.baud_rate;
}

int32_t OMDynamixel::getData(uint8_t id, uint16_t addr, uint8_t length)
{
  return dxl_wb_.itemRead(id, addr, length);
}

std::vector<float> OMDynamixel::getAngle()
{
  int32_t *get_position_ptr = NULL;
  radian_value_.clear();

  if (dxl_wb_.getProtocolVersion() == 2.0)
  {
    get_position_ptr = dxl_wb_.syncRead(PRESENT_POSITION);    

    for (uint8_t index = 0; index < dxl_info_.size; index++)
      radian_value_.push_back(dxl_wb_.convertValue2Radian(dxl_id_.at(index), get_position_ptr[index]));
  }
  else
  {
    for (uint8_t index = 0; index < dxl_info_.size; index++)
      radian_value_.push_back(dxl_wb_.convertValue2Radian(dxl_id_.at(index), dxl_wb_.itemRead(dxl_id_.at(index), PRESENT_POSITION)));
  }

  return radian_value_;
}

std::vector<float> OMDynamixel::getCurrent()
{
  int32_t *get_current_ptr = NULL;
  get_current_ptr = dxl_wb_.syncRead(PRESENT_CURRENT);

  for (uint8_t index = 0; index < dxl_info_.size; index++)
    torque_value_.push_back(dxl_wb_.convertValue2Torque(dxl_id_.at(index), get_current_ptr[index]));

  return torque_value_;
}

int32_t OMDynamixel::convertRadian2Value(uint8_t id, float radian)
{
  return dxl_wb_.convertRadian2Value(id, radian);
}

