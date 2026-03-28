📷 Embedded Vision Acquisition System (STM32 + OV7670)

This project implements a lightweight embedded vision acquisition system on the STM32 Nucleo platform using the OV7670 camera module.

The system demonstrates a simplified camera pipeline, including sensor configuration, frame capture, data processing, and real-time transmission.

🚀 Features
📸 Dual Capture Modes
Snapshot Mode: Capture a single frame on demand using DCMI + DMA
Continuous Mode: Stream frames continuously using DMA-based capture
⚡ Hardware-Accelerated Data Path
Utilizes DCMI + DMA to transfer image data directly to memory
Minimizes CPU involvement and improves real-time performance
🎛 Camera Configuration (SCCB / I2C)
Initializes OV7670 registers (frame rate, QCIF resolution, pixel format)
Supports YCbCr output format
🎞 Grayscale Image Extraction
Parses YCbCr stream and extracts luminance (Y) component
Converts raw camera output into grayscale images
📡 Real-Time UART Streaming
Transmits processed frames to PC via UART
Compatible with external visualization tools (e.g., serial monitor)
🧵 Tearing-Free Frame Handling
Implements DCMI suspend/resume mechanism
Decouples frame capture and transmission to prevent tearing artifacts

⚙️ Hardware Setup
Board: STM32 Nucleo (F4 series recommended)
Camera: OV7670
Interfaces:
DCMI (parallel camera interface)
I2C (SCCB for camera configuration)
UART (data transmission)

The OV7670 outputs pixel data synchronized by PCLK, HSYNC, and VSYNC signals, which are handled by the DCMI peripheral.

🛠 Key Implementation Details
📌 1. Frame Capture
Uses HAL_DCMI_Start_DMA() for both snapshot and continuous modes
DMA transfers pixel data directly into memory buffer
📌 2. Data Format Handling
Camera outputs YCbCr format
Only Y (luminance) is extracted for grayscale visualization
📌 3. Continuous Streaming
Continuous mode uses DMA circular transfer
Frame-ready interrupt triggers data processing and transmission
📌 4. Tearing Avoidance
During transmission:
HAL_DCMI_Suspend() pauses capture
HAL_DCMI_Resume() resumes after processing
Prevents mixing of two frames (screen tearing)
📊 Performance Considerations
DMA eliminates CPU bottleneck in frame acquisition
UART bandwidth becomes the limiting factor for frame rate
Trade-offs:
Higher resolution → lower FPS
Faster baud rate → smoother streaming
🧪 Demo
Snapshot: press trigger to capture a single frame
Continuous: stream grayscale video to PC

Visualization can be done using a custom serial monitor tool.

📌 Future Improvements
Port to Linux V4L2 driver model
Add double buffering for smoother streaming
Replace UART with USB / SPI / SD card logging
Integrate basic image processing (edge detection / thresholding)
