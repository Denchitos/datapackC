scoreboard players set .global.dp.for_test2.__temp3 local 2
scoreboard players operation .global.dp.for_test2.__temp3 local *= .global.dp.for_test2.x local
scoreboard players operation .global.dp.for_test2.__returned local = .global.dp.for_test2.__temp3 local
return 0

