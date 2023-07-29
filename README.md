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

For our proof of concept, we have equipped the prototype with three key sensors: SEN0161 for pH measurement, DS18B20 for water temperature, and MQ135 for NH3 detection. These sensors collectively provide crucial information about the aquatic and plant environments, forming the foundation of our monitoring system.

The ESP32 microcontroller serves as the central processing unit, efficiently gathering sensor data. To transmit this data to the cloud, we employ the MQTT protocol, a lightweight messaging protocol ideal for IoT applications. Leveraging the robust infrastructure of Amazon Web Services (AWS), we utilize AWS IoT Core as the MQTT broker, ensuring secure and reliable data communication between the prototype and the cloud.

Once in the cloud, the data is efficiently stored in AWS Timestream, a scalable and purpose-built time-series database, allowing us to organize and manage the information effectively. Additionally, historical data is archived in an AWS S3 bucket, ensuring a comprehensive dataset for long-term analysis and reference.

To visualize and interpret the data, we utilize Grafana, a powerful analytics and monitoring platform. Grafana enables stakeholders to access real-time and historical data, empowering them to make informed decisions based on dynamic visualizations and dashboards.

Furthermore, to ensure prompt action in critical scenarios, we have implemented alert mechanisms that trigger notifications when specific threshold values are exceeded. This enables swift responses to potential issues, maintaining the system's health and productivity.

### AquaFeeder 

[how it works ...]

