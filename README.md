# DatapackC
Компилятор с собственного DSL в .mcfunction для создания датапаков *Minecraft*

## Структура программы

Код для datapackC делится на следующие секции
```text
  <файл>
  - namespace - пространства имён
  - objective - таблицы scoreboard
  - def < имя функции > - функции
  - inline < имя функции > - то же функции ( разница будет объяснена в отдельном разделе )
  - storage - nbt-структуры
```

### **namespace**

---

namespace - ключевое слово для объявления пространства имён где компилятор будет создавать функции

---

Например, вы в файле напишите следующее
```text
  namespace global:dp/tests
```
Будет означать что компилятор будет создавать все далее объявленные функции в таком виде
```text
datapack/
└── data/
   └── global/
      └── function/
         └── dp/
            └── tests/
                ├── func_1
                ├── func_2
                ├── ...
                └── func_n
```
Т.е это просто префикс, который вы указываете перед именем функции при её вызове в майнкрафт.

Например функция func_k будет вызываться в майнкрафте так
```text
  function global:dp/tests/func_k
           └─────────────┘
            - это то, что вы указали после namespace
```

Пример других корректных пространств имён
```text
  namespace global:dp
  namespace minecraft
  namespace minecrst:functions/print
```
... и не корректных
```text
  namespace global:dp/ # <-- оканчивается на '/', это недопустимо
  namespace minecraft/dp # <-- после первого слово обязано быть ':'
  namespace minecraft: # <-- после ':' должно идти следующее слово
```

---

**И да, комментарии в моём языке обозначаются через '#'. Комментироваться будет всё от '#' до конца строки**

---

## **objective**

---

objective - ключевое слово, которое указывает создать scoreboard в *Minecraft*

---

Например, вы в файле напишите следующее
```text
  objective myscore
```
И что это значит?

Предположим вы создаёте objective в пространстве global:dp
```text
  namespace global:dp

  objective myscore
```

Это означает что в датапаке будет создан следующий scoreboard
```text
  scoreboard objectives add global.dp.myscore dummy
```
Это строка будет добавлена в load.mcfunction файл - который будет запускаться при загрузке мира *Minecraft* или после /reload

---

Как работать с этими objectives будет показано в другом разделе
