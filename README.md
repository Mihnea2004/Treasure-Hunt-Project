This project aims to simulate a Treasure Hunt game, required at the Operating Systems Course. It is implemented in 3 Phases:

Phase 1 - treasure_manager.c: You can test the code by writing the instructions below in your terminal:
  1. gcc -Wall -o exe treasure_manager.c
  2. ./exe add Hunt001 [then enter all information required, you can add more treasures in one Hunt]
  3. ./exe list Hunt001
  4. ./exe view Hunt001 1 [1 = treasure ID]
  5. ./exe remove_treasure Hunt001 1 [1 = treasure ID]
  6. ./exe remove_hunt Hunt001

Phase 2 - treasure_hub.c: You can test the code by writing the instructions below in your terminal:
  1. gcc -Wall -o exe treasure_hub.c
  2. ./exe [then you will see the command prompt, written in purple]
  3. prompt > startMonitor
  4. prompt > listHunts
  5. prompt > listTreasures Hunt001
  6. prompt > viewTreasure Hunt001 1 [1 = treasure ID]
  7. prompt > stopMonitor
  8. prompt > exit

Phase 3 - score_calculator.c + treasure_hub.c: It implements the same features as Phase 2, but, in addition, there is a new command, calulcateScore, which creates a process for each existing hunt that calculates and outputs the scores of users in that hunt. You can test the code by writing the instructions below in your terminal:
  1. gcc -Wall -o exe treasure_hub.c
  2. ./exe [then you will see the command prompt, written in purple]
  3. prompt > startMonitor
  4. prompt > listHunts
  5. prompt > listTreasures Hunt001
  6. prompt > viewTreasure Hunt001 1 [1 = treasure ID]
  7. prompt > calculateScore
  8. prompt > stopMonitor
  9. prompt > exit
