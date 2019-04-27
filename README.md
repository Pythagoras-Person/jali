So I never found out the name of this game, I just saw it in some Anime and in a Super Mario 64 DS Minigame so I think it's from Japan. It was an Project to learn how to use SDL2 with C and helped me alot with understanding C.

The File Format:
----------------
  - this is a very simple ASCII based format wich dezimal input
  - one Entry(Line) consists out of 3 Numbers, seperated by just a comma:
      - the first Number has to be in range of 0 - 2 and tells the programm on wich vertical line your line should start
      - the secound Number has to be in range of 1 - 31 and tells the programm the vertical position at the defined vertical line
      - the third Number does the same like the secound but at the end of your line at the following vertical line
  - every Entry ends with a '\n' (the last line should be emty)

   0   1   2   3
1  |   |   |   |
2  |---|   |   |
3  |   |   |   |
4  |   |   |   |
5  |   |   |   |
6  |   |   |   |

the above line would have the following Entry
  -> 0,2,2

The Transform script:
---------------------
  - this is a simple Python3 script to optimise your Map-file
  - the value shift, shifts the whole map vertical (positive and negativ nubers work), the value 0 does nothing
  - the value scale, scales the whole map vertical (only positiv numbers work), the value 1 does nothing

Using the binary:
-----------------
  - to use the binary output from gcc, you have to give 2 arguments
  - argument 1 -> map file (.lmp means: line map)
                                        ^    ^ ^
  - argument 2 -> a nuuber between 0 - 3 to tell the programm where to start (optinal, default its set to 0)
