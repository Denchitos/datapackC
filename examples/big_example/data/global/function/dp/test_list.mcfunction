data modify storage global:dp/test_list intlist set value [{},{},{},{}]
data modify storage global:dp/test_list intlist[0] set value 5
data modify storage global:dp/test_list intlist[1] set value 4
data modify storage global:dp/test_list intlist[2] set value 3
data modify storage global:dp/test_list intlist[3] set value 2
tellraw @a { storage : "global:dp/test_list",nbt:"intlist" }
execute store result score .global.dp.test_list.__temp4 local run data get storage global:dp/test_list intlist
scoreboard players operation .global.dp.test_list.len local = .global.dp.test_list.__temp4 local
data modify storage global:dp/test_list item set from storage global:dp/test_list intlist[0]
scoreboard players set .global.dp.test_list.a local 5
data modify storage global:dp/print str set value "Ожидается ...,2,5"
function global:dp/print
data modify storage global:dp/test_list intlist append from storage global:dp/test_list item
tellraw @a { storage : "global:dp/test_list",nbt:"intlist" }
data modify storage global:dp/print str set value "Ожидается 5,5..."
function global:dp/print
data modify storage global:dp/test_list __temp5 set value 5
data modify storage global:dp/test_list intlist prepend from storage global:dp/test_list __temp5
tellraw @a { storage : "global:dp/test_list",nbt:"intlist" }
data modify storage global:dp/print str set value "Ожидается 5,5,97..."
function global:dp/print
data modify storage global:dp/test_list __temp6 set value 97
data modify storage global:dp/test_list intlist insert 2 from storage global:dp/test_list __temp6
tellraw @a { storage : "global:dp/test_list",nbt:"intlist" }

