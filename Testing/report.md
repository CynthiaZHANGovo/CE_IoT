\## The sensor refresh rate is limited. 



The HC-SR04 requires a waiting period for the sound wave to return (the maximum possible time is 30ms to 60ms). 

If it call the distance measurement function every time within the loop and then use delay(30), 

then sampling interval is approximately 30 to 40 milliseconds. 

Therefore it can only capture about 25 to 30 points per second. 

When I move my hand rapidly, the speed changes too quickly, and the instantaneous changes in the middle will be "undersampled".

As a result, the calculated speed value will be discontinuous or underestimated. 



\##  Physical noise \& Sound wave interference 



The angle of ultrasonic reflection has a significant impact on the results (especially when the hand is not perpendicular to the surface). 

When the distance changes rapidly, the reflected signal will attenuate or split, causing the pulseIn() function to read incorrect values. 

The "noise reduction" feature built into the HC-SR04 is rather limited. The signal fluctuation can be quite significant within a short period of time.



\##  Limitations of the code calculation method 



I used:

speed = abs(distance - lastDistance) / (float)timeDiff \* 1000.0; 



This is actually the average speed between the two frames.

If the object accelerates or decelerates during the process, the algorithm won't be able to capture the details; it will only see the difference between the two measurements.

This is like taking sample photos ;(  it's not a true continuous speed sensing.

