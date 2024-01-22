# FPGA-Snake

## Game Description
"Snake" is a classic arcade game where the player controls a snake moving around a screen with one objective: eat the randomly appearing fruit on the screen. The snake grows in length whenever a fruit is eaten, with the main difficulty being to avoid colliding with the snake's own tail. As the snake consumes fruits, the player’s score increases but this causes the snake to grow longer which increases the game’s difficulty. 

I also added an extra feature to the game where the speed of the snake increases as the player’s score increases. This further increases the game’s difficulty as the game goes on. 

The snake game is a classic game that has been played for decades, providing entertainment whenever it is needed. By creating this on the FPGA, we hope to create a handheld way to play the “Snake” game at any time for the user. This turns the FPGA into an entertainment device and creates a useful project.


## System Design 
Snake Game on the Digilent Basys 3 FPGA board using Microblaze and the following peripheral PMODs: 
* OLEDrgb -> display used to display the snake game
* JSTK -> joystick used to control the snake's movements
The game also utilizes the following built in peripherals on the Basys 3: 
* Seven segment display -> displays user's score
* Button -> resets the user's score and restarts the game

<img width="684" alt="Screen Shot 2024-01-22 at 11 58 28 AM" src="https://github.com/sdcarlson/FPGA-Snake/assets/66243744/8dd790de-8424-4a21-82eb-710f25f7ebb0">
* Image of FPGA and peripherals setup for the game

