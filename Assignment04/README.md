This project took quite a long time because I was having a hard time parsing the f values from the obj file. I loaded all the v's easily since they were always the same format.

I had the issues with loading the f's since there were so many different ways of formating the obj files. At first I didnt know about .peek() so I was loading
the characters one at a time and kept testing for different possibilities. It was mostly just trial and error until it worked after I saw that there is a .peek()
function. 

Add objects through cmd prompt: ./Matrix object.obj
Controls:
a/LMB - reverse rotation
RMB - menu
Left arrow - orbit counter-clockwise
right arrow - orbit clockwise
