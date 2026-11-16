scoreboard players set .global.dp.no_func.__is_returned local 0
function global:dp/no_func/__cb__0
execute unless score .global.dp.no_func.__is_returned local matches 0 run return 0
scoreboard players operation .global.dp.no_func.__returned local = .global.dp.no_func.a local
return 0

