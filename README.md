# autoguider

This is the source code for the autoguider software for the Liverpool Telescope.

This software drives a CCD/CMOS camera. In normal operation:
* It takes a field image (full-frame image).
* It finds potential guide stars in the image.
* It selects a guide star and a suitable exposure length.
* It windows the CCD around the guide star.
* It loops:
  * Taking guide frames
  * Extracting a centroid
  * Informing the SDB and TCS of the guide star position (so they can move the telescope to correct tracking errors).

The autoguider can be commanded from the TCS using the TTL CIL command interface. There is also a telnet based
engineering interface which can be used to break down the above procedure into small steps, and control things like the CCD camera temperature.

## Directory Structure

* **c**             The main C control software that forms the autoguider process/program.
* **ccd**           The autoguider's camera control drivers, and generic detector interface.
* **commandserver** A C library and test programs, to provide a telnet/socket based way to control the autoguider software.
* **docs**          A directory containing the LaTeX source to generate the documentation.
* **include**       The directory containing header files for the C layer modules forming the main autoguider process/program.
* **java**          The root directory of the Java software for an autoguider GUI and Java client-side command implementations.
* **ngatcil**       A C library and test programs that allow the sending and receiving of TTL CIL (communication infrastructure library) packets to and from the telescope TCS (telescope control system) and SDB (status database). The telescope uses CIL to command the autoguider and receive telemetry (guide packets) from the autoguider.
* **scripts**       Deployment and engineering scripts.
* **txt**           Various text files with instructions, notes and information used when creating the software.

The Makefile.common file is included in Makefile's to provide common root directory information.

## Dependencies / Prerequisites

* The relevant camera SDK must be installed.
* The log_udp repo/package must be installed: https://github.com/LivTel/log_udp
* The eSTAR config repo/package must be installed.
* The ngat repo/package must be installed: https://github.com/LivTel/ngat
* Parts of the libdprt real-time data reduction library must be installed (specifically, the object detection library).
* The ngatastro repo/package must be installed.
* The software can only be built from within an LT development environment
