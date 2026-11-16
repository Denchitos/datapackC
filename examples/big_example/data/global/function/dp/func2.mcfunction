data modify storage global:dp/func2 y set value 27
data modify storage global:dp/func2 c.a set value "Hello"
data modify storage global:dp/func2 c.b set value "World"
data modify storage global:dp/func2 c.c set value "!"
data modify storage global:dp/func2 b.a set value 5
data modify storage global:dp/func2 b.b set from storage global:dp/func2 c
data modify storage global:dp/func2 b.c set value 5
scoreboard players set @s global.dp.dms 15
data modify storage global:dp/func2 zoomba set from storage global:dp/func2 b.b.c
data modify storage global:dp/for_test b set from storage global:dp/func2 b
function global:dp/for_test
data modify storage global:dp/func2 hun set from storage global:dp/for_test __returned.a
scoreboard players set .global.dp.for_test2.x local 5
function global:dp/for_test2
scoreboard players operation .global.dp.func2.amo local = .global.dp.for_test2.__returned local
data modify storage global:dp/func2 xyz set from storage global:dp/func2 b
scoreboard players set @s global.dp.dms 5
execute as @s at @s run function global:dp/say
data modify storage global:dp/func2 d.a set value 5
data modify storage global:dp/func2 d.b.a set value "Hello"
data modify storage global:dp/func2 d.b.b set value "World"
data modify storage global:dp/func2 d.b.c set value "!"
data modify storage global:dp/func2 d.c set value 5
function global:dp/testual
data modify storage global:dp/func2 ano set from storage global:dp/testual __returned[0]
data modify storage global:dp/print str set value "There is called b.func()"
function global:dp/print
data modify storage global:dp/print str set from storage global:dp/func2 b.b.a
function global:dp/print
scoreboard players set .global.dp.print_score.a local 10
function global:dp/print_score
data modify storage global:dp/func2 axid set from storage global:dp/func2 b
data modify storage global:dp/func2 zzz set value [{},{},{}]
data modify storage global:dp/func2 zzz[0] set value 55
data modify storage global:dp/func2 zzz[1] set from storage global:dp/func2 y
data modify storage global:dp/func2 zzz[2] set value 66
data modify storage global:dp/func2 zzz2 set value [{},{}]
data modify storage global:dp/func2 zzz2[0] set value []
data modify storage global:dp/func2 zzz2[0] set from storage global:dp/func2 zzz
data modify storage global:dp/func2 zzz2[1] set value [{},{},{}]
data modify storage global:dp/func2 zzz2[1][0] set value 5
data modify storage global:dp/func2 zzz2[1][1] set value 5
data modify storage global:dp/func2 zzz2[1][2] set value 5
data modify storage global:dp/print str set value "There is called aboba.func()"
function global:dp/print
data modify storage global:dp/func2 x set from storage global:dp/func2 zzz2[0][0]
data modify storage global:dp/func2 bets set value [{}]
data modify storage global:dp/func2 bets[0] set from storage global:dp/func2 b
data modify storage global:dp/func2 aboba.list set value []
data modify storage global:dp/func2 aboba.list set from storage global:dp/func2 bets
data modify storage global:dp/print str set from storage global:dp/func2 aboba.list[0].b.a
function global:dp/print
scoreboard players set .global.dp.print_score.a local 10
function global:dp/print_score

