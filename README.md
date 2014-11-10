WinTimer
========

A simple timer for Windows. It's technically written in C++, but pretty much all of it is Windows specific, so it feels different than most C++ programs.

Features
========

* Left click to make the timer start/stop.
* Right click to reset the time. This won't stop or start it.
* Type numbers to set the timer to count down from some time. It will beep when it reaches 00:00:00.
    * It starts setting numbers from the left, goes to the right, and at the end jumps back to the left. It will ignore invalid input. 
    * For example, typing 0002 will set the timer to 2 minutes. 0000000002 will also set the timer to 2 minutes.
