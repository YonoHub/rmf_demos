/*
 * Copyright (C) 2020 Open Source Robotics Foundation
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
 *
*/

#include "RmfPanel.hpp"

#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

namespace rmf_rviz_plugin {

void RmfPanel::create_layout()
{
  // Layout for delivery request
  // HLayout 1
  QHBoxLayout* layout_delivery1 = new QHBoxLayout;
  layout_delivery1->addWidget(new QLabel("Delivery Request"));
  layout_delivery1->addStretch();

  // HLayout 2
  QHBoxLayout* layout_delivery2 = new QHBoxLayout;
  layout_delivery2->addWidget(new QLabel("Task ID:"));
  _delivery_task_id_editor = new QLineEdit;
  _delivery_task_id_editor->setFixedWidth(100);
  layout_delivery2->addWidget(_delivery_task_id_editor);

  layout_delivery2->addWidget(new QLabel("Robot:"));
  _delivery_robot_editor = new QLineEdit;
  _delivery_robot_editor->setFixedWidth(100);
  layout_delivery2->addWidget(_delivery_robot_editor);
  layout_delivery2->addStretch();

  // HLayout 3
  QHBoxLayout* layout_delivery3 = new QHBoxLayout;
  layout_delivery3->addWidget(new QLabel("Pickup:"));
  _delivery_pickup_editor = new QLineEdit;
  _delivery_pickup_editor->setFixedWidth(100);
  layout_delivery3->addWidget(_delivery_pickup_editor);

  layout_delivery3->addWidget(new QLabel("Dropoff:"));
  _delivery_dropoff_editor = new QLineEdit;
  _delivery_dropoff_editor->setFixedWidth(100);
  layout_delivery3->addWidget(_delivery_dropoff_editor);
  layout_delivery3->addStretch();

  //HLayout 4
  QHBoxLayout* layout_delivery4 = new QHBoxLayout;
  _delivery_button = new QPushButton(this);
  _delivery_button->setText("Request Delivery");
  layout_delivery4->addWidget(_delivery_button);
  layout_delivery4->addStretch();

  QVBoxLayout* layout_delivery = new QVBoxLayout;
  layout_delivery->addStretch();
  layout_delivery->addLayout(layout_delivery1);
  layout_delivery->addLayout(layout_delivery2);
  layout_delivery->addLayout(layout_delivery3);
  layout_delivery->addLayout(layout_delivery4);
  setLayout(layout_delivery);

  // Initialize text fields
  _delivery_task_id_editor->setText(_delivery_task_id);
  _delivery_pickup_editor->setText(_delivery_pickup);
  _delivery_dropoff_editor->setText(_delivery_dropoff);
  _delivery_robot_editor->setText(_delivery_robot);

  // QT signal connections
  connect(_delivery_task_id_editor,
      SIGNAL(editingFinished()), this, SLOT(update_delivery_task_id()));
  connect(_delivery_robot_editor,
      SIGNAL(editingFinished()), this, SLOT(update_delivery_robot()));
  connect(_delivery_pickup_editor,
      SIGNAL(editingFinished()), this, SLOT(update_delivery_pickup()));
  connect(_delivery_dropoff_editor,
      SIGNAL(editingFinished()), this, SLOT(update_delivery_dropoff()));
  connect(_delivery_button,
      SIGNAL(clicked()), this, SLOT(request_delivery()));
}

RmfPanel::RmfPanel(QWidget* parent)
: rviz_common::Panel(parent),
  _delivery_task_id("task#42"),
  _delivery_robot("magni"),
  _delivery_pickup("pantry"),
  _delivery_dropoff("hardware_2")
{
  _node = std::make_shared<rclcpp::Node>("rmf_panel_plugin");

  _delivery_pub = _node->create_publisher<Delivery>(
    "/delivery_requests",
    rclcpp::QoS(10));

  _thread = std::thread([&](){rclcpp::spin(_node);});
  
   create_layout();

  _has_loaded = true;
}

RmfPanel::~RmfPanel()
{
  if (_has_loaded)
  {
    _thread.join();
    rclcpp::shutdown();
  }
}

void RmfPanel::set_delivery_task_id(const QString& value)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _delivery_task_id = value;
  Q_EMIT configChanged();
}

void RmfPanel::set_delivery_pickup(const QString& value)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _delivery_pickup = value;
  Q_EMIT configChanged();
}

void RmfPanel::set_delivery_dropoff(const QString& value)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _delivery_dropoff = value;
  Q_EMIT configChanged();
}

void RmfPanel::set_delivery_robot(const QString& value)
{
  std::lock_guard<std::mutex> lock(_mutex);
  _delivery_robot = value;
  Q_EMIT configChanged();
}

void RmfPanel::update_delivery_task_id()
{
  set_delivery_task_id(_delivery_task_id_editor->text());
}

void RmfPanel::update_delivery_pickup()
{
  set_delivery_pickup(_delivery_pickup_editor->text());
}

void RmfPanel::update_delivery_dropoff()
{
  set_delivery_dropoff(_delivery_dropoff_editor->text());
}

void RmfPanel::update_delivery_robot()
{
  set_delivery_robot(_delivery_robot_editor->text());
}

void RmfPanel::request_delivery()
{
  Delivery delivery;

  std::lock_guard<std::mutex> lock(_mutex);
  _delivery_task_id = _delivery_task_id == "" ? "task#42" : _delivery_task_id;

  delivery.task_id = _delivery_task_id.toStdString();
  delivery.pickup_place_name = _delivery_pickup.toStdString();
  delivery.dropoff_place_name = _delivery_dropoff.toStdString();

  _delivery_pub->publish(delivery);
}

void RmfPanel::load(const rviz_common::Config& config)
{
  rviz_common::Panel::load(config);
  QString delivery_task_id;
  QString delivery_robot;
  QString delivery_pickup;
  QString delivery_dropoff;

  if (config.mapGetString("delivery_task_id", &delivery_task_id))
  {
    _delivery_task_id_editor->setText(delivery_task_id);
    update_delivery_task_id();
  }
}

void RmfPanel::save(rviz_common::Config config) const
{
  rviz_common::Panel::save(config);
  config.mapSetValue("delivery_task_id", _delivery_task_id);
}

} // namespace rmf_rviz_plugin

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS(rmf_rviz_plugin::RmfPanel, rviz_common::Panel)
