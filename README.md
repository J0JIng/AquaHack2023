# IEEE NUS x IEEE NTU Aquaponics Hackathon 2023

This repository contains the relevant code and prototype for the Aquaponics Hackathon 2023, hosted by the IEEE NUS X IEEE NTU student branch.

---
### Overview:
Aquaponics Monitoring System for Tilapia and Asian Leafy Greens Farm with an emphasis on inclusivity and social impact.  

### Introduction:
Aquaponics is a sustainable agricultural system that combines aquaculture with hydroponics. It creates a mutually beneficial ecosystem where fish waste provides nutrients for the plants, and the plants help filter and clean the water for the fish. While aquaponics offers numerous benefits, it requires meticulous care to maintain the delicate balance between fish and plants. To streamline the monitoring process and ensure optimal conditions for both Tilapia fish and Asian leafy greens, we have developed an Aquaponics Monitoring System called CloudSensor.

In this proposal, we outline our vision to enhance the social impact of our Aquaponics Monitoring System, CloudSensor, by also introducing AquaFeeder focused on positively contributing to the community, with a particular emphasis on children in hospitals. This aims to improve the well-being of the children, promote environmental awareness, and offer valuable educational experiences.

### Key Objectives:

1. Real-time Monitoring:
The system aims to provide real-time data on essential parameters to closely monitor the health and well-being of the fish and plants.

2. Automation:
Automation features are integrated to reduce manual workload and ensure consistent environmental control.

3. Data Analysis:
The collected data is analyzed to gain insights into the system's performance, enabling informed decision-making for improvements.

### CloudSensor Prototype:

Our proof of concept includes the integration of three key sensors in the prototype, each playing a crucial role in our comprehensive monitoring system. The SEN0161 sensor enables accurate pH measurement, providing valuable insights into the water's acidity or alkalinity. To monitor water temperature variations, we have incorporated the DS18B20 sensor, ensuring we track the aquatic environment effectively. Moreover, we have implemented a color-changing sticker that indicates the concentration of free ammonia in parts per million (ppm), allowing us to assess water quality easily.

To complement this sensor array, we plan to use the ESP32 CAM to monitor the ammonia sticker actively. The ESP32 CAM will capture snapshots, providing visual information about the ammonia sticker's changes and helping us understand the dynamics of ammonia concentration over time.

Additionally, we may attach other peripheral sensors, such as lux sensors, to gather a variety of readings and further enhance our understanding of both aquatic and plant environments. These sensors collectively form the foundation of our monitoring system, empowering us to make informed decisions and take necessary actions to maintain optimal conditions for aquatic life and plants.

The ESP32 microcontroller serves as the central processing unit, efficiently gathering sensor data. To transmit this data to the cloud, we employ the MQTT protocol, a lightweight messaging protocol ideal for IoT applications. Leveraging the robust infrastructure of Amazon Web Services (AWS), we utilize AWS IoT Core as the MQTT broker, ensuring secure and reliable data communication between the prototype and the cloud.

Once in the cloud, the data is efficiently stored in AWS Timestream, a scalable and purpose-built time-series database, allowing us to organize and manage the information effectively. Additionally, historical data is archived in an AWS S3 bucket, ensuring a comprehensive dataset for long-term analysis and reference.

To visualize and interpret the data, we utilize Grafana, a powerful analytics and monitoring platform. Grafana enables stakeholders to access real-time and historical data, empowering them to make informed decisions based on dynamic visualizations and dashboards.

Furthermore, to ensure prompt action in critical scenarios, we have implemented alert mechanisms that trigger notifications when specific threshold values are exceeded. This enables swift responses to potential issues, maintaining the system's health and productivity.

### AquaFeeder 

[how it works ...]

