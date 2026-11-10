data modify storage global:dp/func2 y set value 27
data modify storage global:dp/func2 c.a set value "Hello"
data modify storage global:dp/func2 c.b set value "World"
data modify storage global:dp/func2 c.c set value "!"
data modify storage global:dp/func2 b.a set value 5
data modify storage global:dp/func2 b.b set from storage global:dp/func2 c
data modify storage global:dp/func2 b.c set value 5
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
data modify storage global:dp/func2 zzz set value [0,0,0]
data modify storage global:dp/func2 zzz[0] set value 55
data modify storage global:dp/func2 zzz[1] set from storage global:dp/func2 y
data modify storage global:dp/func2 zzz[2] set value 66
data modify storage global:dp/func2 zzz2 set value [0,0]
data modify storage global:dp/func2 zzz2[0] set from storage global:dp/func2 zzz
data modify storage global:dp/func2 zzz2[1] set value [0,0,0]
data modify storage global:dp/func2 zzz2[1][0] set value 5
data modify storage global:dp/func2 zzz2[1][1] set value 5
data modify storage global:dp/func2 zzz2[1][2] set value 5
data modify storage global:dp/func2 zzz_item set from storage global:dp/func2 zzz[5]
execute store result score .global.dp.func2.__temp4 local run data get storage global:dp/func2 zzz
scoreboard players operation .global.dp.func2.zzz_len local = .global.dp.func2.__temp4 local
data modify storage global:dp/func2 zzz_item set from storage global:dp/func2 zzz2[1][0]
scoreboard players set .global.dp.func2.__is_returned local 0
function global:dp/func2/__cb__0
execute unless score .global.dp.func2.__is_returned local matches 0 run return 0
scoreboard players set .global.dp.func2.__returned local 10
return 0

