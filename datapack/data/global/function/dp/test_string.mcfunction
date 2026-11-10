scoreboard players set .global.dp.print_score.a local 5
function global:dp/print_score
data modify storage global:dp/print str set value "String:"
function global:dp/print
data modify storage global:dp/test_string b set value "Hello World!"
data modify storage global:dp/print str set from storage global:dp/test_string b
function global:dp/print
scoreboard players set .global.dp.compare.a local 5
scoreboard players set .global.dp.compare.b local 6
function global:dp/compare
scoreboard players set .global.dp.no_func.a local 2
function global:dp/no_func
scoreboard players set .global.dp.print_score.a local 10
function global:dp/print_score
scoreboard players set .global.dp.test_string.c local 10
scoreboard players operation .global.dp.print_score.a local = .global.dp.test_string.c local
function global:dp/print_score
tellraw @a [ "b string is ", { storage : "global:dp/test_string",nbt:"b",interpret:true }]
scoreboard players set .global.dp.no_func.a local 4
function global:dp/no_func
scoreboard players operation .global.dp.test_string.c local = .global.dp.no_func.__returned local

