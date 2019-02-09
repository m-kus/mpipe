# MPipe

[![Build Status](https://travis-ci.org/m-kus/mpipe.svg?branch=master)](https://travis-ci.org/m-kus/mpipe)
[![Made With](https://img.shields.io/badge/made%20with-cpp-red.svg?)](https://www.python.org)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Это библиотека на C++ для быстрого создания торговых систем из кубиков - модулей `Module`. 
Они ничего не знают друг про друга и общаются с помощью большой структуры - состояния `State`.
Есть готовые модули, функционал которых покрывает большинство задач, однако можно легко реализовывать свои, на C++ или Python (без потери производительности). 
Процесс создания приложения заключается в инициализации необходимых модулей и их объединения в конвейеры - пайпы `Pipe`.

# Быстрый старт

Забираем из последней удачной (зеленой) сборки артифакты:

* MQL5, для работы с метатрейдером, файлы надо аналогично разложить по папочкам
* Python inplace, для быстрой отладки, надо закинуть в папку с вашим скриптом
* Python wheel, можно установить так: `pip install <wheel_name>.whl`

## Хочу написать страту на питоне

Идем по порядку, сперва нужно создать инструменты, которые му будем использовать.
Обычно они есть в библиотеке предопределенных инструментов `SecurityStore`, если нет - пните меня или добавьте сами.

```python
from pype import *
ss = SecurityStore()
```

Будем строить индикатор и торговать по сишке:

```python
si = ss.Create('Si', settings="trades,finam,book1", leg_factor=1)
```

leg_factor это на что мы будем домножать размер и направление ордера (он может быть 0, если мы не хотим его торговать), флаги означают, что мы хотим получать сделки, исторические данные, стакан глубиной 1.

Ура! Теперь можно создать состояние.

```python
state = State([si])
```

Нам нужны исторические данные с Финама, таймфрейм 1 минута, с предыдущего торгового дня, закрытия свечи:

```python
finam = Finam(
    timeframe=FinamTimeFrame.Min1,
    day_shift=1,
    bar_open=False)
```

Котировки c минимальной частотой тика 5 секунд:

```python
mcast = Mcast([], quartz_period=5000)
```

Ограничение по времени торговли:

* Начинаем торговать в 7
* Не открывать позицию после 10
* Принудительное закрытие в 11

```python
hours = ModHours([
	ModHoursRule("19:00:00", Side.All, Action.Open),
    ModHoursRule("22:00:00", Side.All, Action.Close),        
    ModHoursRule("23:00:00", Side.All, Action.ForceClose)
])
```

Хотим, чтобы позиция закрывалась по времени через час (этот модуль генерирует ордеры, нужно ставить его перед другими модулями, создающими ордеры):

```python
tclose = ModTimeClose(timeout=3600000)
```

Теперь модуль, который будет по сигналу создавать торговые ордеры:

```python
ordgen = ModOrderGen(lot_size=1)
```

Далее цепочка фильтров по объему и цене:

* Не больше, чем позволяет ограничение по позе
* Не больше, чем есть в стакане, но не меньше заданного количества
* По лучшей цене + один пункт вглубь

```python
poslmt = ModPosLimit(max_pos=1)
market_px = ModMarketPx(min_qty=1, slippage=-1)
```

Наконец, торговый шлюз! Возьмем для начала виртуальное исполнение:

```python
executor = ModVirtExec(
    trade_timeout=1000, 
    cancel_timeout=3000, 
    trades_file='trades.csv', 
    random_unfill=False
)
```

А теперь самое главное, сама стратегия. Мы должны обработать котировки и передать торговый сигнал.

```python
class Strategy(Module):
    
    def __init__(self):
        super(Strategy, self).__init__()
        
    def Mutate(self, state: State)
    	pass
```

Это базовый каркас, теперь начнем его наполнять. Для примера возьмем простой алгоритм: покупаем ниже скользящей средней, продаем выше. Нам потребуется индикатор:

```python
self.rol_mean = RollingMean(window=15, timeframe=60000)  # 15 минут
```

Каждый вызов `Mutate` это тик, обновляем индикатор:

```python
if state.securities[0].trd_ts > 0:  # если цена последней сделки валидная
	self.rol_mean.Update(state.signal.ts, state.securities[0].trd_px)  # то обновляем
```

Сама логика:

```python
si = state.securities[0]
if self.rol_mean.Good():  # прогрелся, значение валидное
    mean = self.rol_mean.calculate()
    if si.bid > mean + 100:
        state.signal.action = Action.Open
        state.signal.side = Side.Sell
    elif si.ask < mean - 100:
        state.signal.action = Action.Open
        state.signal.side = Side.Buy
```

Отлично! Надо все это теперь соединить вместе и запустить.

```python
import time

strategy = Strategy()

executor.Start(state, [])
finam.Start(state, [strategy])  # запускаем в начале, чтобы прогреть индикатор
mcast.Start(state, [strategy, tclose, hours, ordgen, poslmt, market_px, executor])

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    pass

executor.Stop()
mcast.Stop()
```

Обязательно надо останавливать активные модули в конце!

## Хочу прикрутить сигналы из метака

Общение будет реализовано с помощью 0mq сокетов, для этого есть специальный модуль. В нем из сокета считывается бинарная структура `Signal`:

```c++
struct Signal
{
  	int64_t ts;  // время сигнала в миллисекундах
  	Side side;  // (int32_t) направление ордеров: покупка, продажа
	Action action;  // (int32_t) действие: открытие, закрытие, пропуск
};
```

Варианты:

- Открыть лонг `ActionOpen` `SideBuy`
- Открыть шорт `ActionOpen` `SideSell`
- Закрыть лонг `ActionClose` `SideSell`
- Закрыть шорт `ActionClose` `SideBuy`
- Закрыть позицию `ActionClose` `SideAll`
- Ничего не делать `ActionBypass` `SideAll`

Специальные случаи:

- Аварийное завершение пайпа `ActionTerminate`
- Принудительное закрытие позиции `ForceClose` (по времени, в конце торгового дня)

Внутри модуля идет проверка, насколько старый сигнал записан в память. 
Если значение больше допустимого таймаута, то считается, что сигнала нет (`ActionBypass`).

На стороне метатрейдера нужно писать в сокет соответствующие значения (не забываем разложить нужные фалы по папочкам):

```c++
#include <ModZmqSignalPub.mqh>

input string forwarder_tx = "tcp://127.0.0.1:42001";
ModZmqSignalPub pub;

int OnInit()
{
   if (pub.Stat(forwarder_tx) != 0)
      return INIT_FAILED;
         
   return INIT_SUCCEEDED;
}

void OnTick()
{
  	if (OpenLong())
    {
      	pub.SendSignal(ActionOpen, SideBuy);
    }
  	else
    {
      	pub.SendSignal(ActionBypass, SideAll);
    }   		
}
```



## Хочу торговать синтетик

В случае синтетической позиции, у нас появляется несколько "ног". Соотношение между ними регулируется с помощью коэффициента `leg_factor`. Например, для торговли фьючерсным спредом значения будут 1 и -1.

Также появится несколько дополнительных модулей:

* Модуль, выравнивающий размер ордеров `ModEqualizer`
* Модуль, выравнивающий позицию, в случае неисполнения части ордеров `ModBalancer`, он работает только если нет висящих неисполненных ордеров

```python
equalizer = ModMcastEqualizer()
balancer = ModMcastBalancer()

executor.Start(state, [balancer, poslmt, market_px, executor])  # вызывается на каждую сделку
mcast.Start(state, [strategy, ..., poslmt, market_px, equalizer, executor]) # перед исполнением
```



## Хочу протестировать страту

Нет проблем, берем модуль Финама, задаем в нем количество торговых дней и запускаем боевой пайп. 
Единственное, нужно заменить исполнение `ModMarketPx` на `ModTrdPx`:

```python
trdpx = ModTrdPx(spread_pt=2)

finam.Start(state, [strategy, ..., trdpx, executor])
```



## Часто возникающие проблемы

#### Кастомный модуль падает при вызове Mutate

* Проверьте наличие в конструкторе  `super(<YourModuleName>, self).__init__()`
* Оберните тело функции в `try except`

#### Не торгует :(

* Убедитесь, что в сигнале не нулевой таймстамп `ts`
* `leg_factor` должен быть отличным от нуля

# Как все работает

## Модули

Модули — объекты, генерирующие (активные) или преобразующие (пассивные) состояние `State`. 
Пайп представляет собой список модулей, которые выполняются слева направо. 
Пайп есть у каждого активного модуля (может быть пустой).
Когда активный модуль принимает решение о запуске пайпа, он ждет, пока освободится состояние, блокирует его (чтобы одновременно выполнялся только один пайп) и запускает обработку сигнала.

Очень важно, чтобы активный модуль не блокировал основной поток приложения. Исключение делается только для загрузки исторических данных и тестирования.

Пассивные модули должны реализовывать метод `Mutate` , принимающий в качестве аргумента указатель на состояние `State`.

Запуск и остановка активного модуля осуществляется с помощью методов `Start` (передается указатель на состояние и пайп) и `Stop` соответственно.
Модуль может переопределить методы `StartImpl` и `Stop` при необходимости.
Для запуска пайпа нужно получить состояние с помощью  `GetState`, далее заблокировать его, вызвать `ExecutePipe` и разблокировать состояние.



### Пишем пассивный модуль

С++

```c++
struct PassiveModule : public Module
{
private:  
  	int64_t int_param_;
  	std::string string_param_;
 
public:  
  	PassiveModule(int64_t some_int_param,  // все параметры можно передать в конструкторе
                  const std::string& some_string_param)
      	: int_param_(some_int_param)
    	, string_param_(some_string_param)
  	{
  	}
  
	void Mutate(State* state) override
	{	
		state->signal.action = Action::Open;
    	state->signal.side = Side::Buy;
	}
};
```

Python

```python
class PassiveModule(Module):
    
    def __init__(self, some_param, another_param):        
        super(PassiveModule, self).__init__()  # без этого не будет работать!
        self.param = some_param
        self.another_param = another_param

	def Mutate(self, state: State):  # не забываем про self
		state.signal.action = Action.Open
    	state.signal.side = Side.Buy
```



### Пишем активный модуль

В С++ для блокировки состояние можно использовать удобный класс `ScopedState`, который работает в пределах видимости. Или можно использовать методы примитива синхронизации:

* `lock` ждет освобождения ресурса и блокирует ресурс
* `unlock` освобождает ресурс
* `try_lock` пытается захватить ресурс, при неудаче возвращает _false_ иначе _true_

С++

```c++
class ActiveModule : public Module
{
private:
  	int64_t int_param_;
  
	void SomeCallbackFromThread(int64_t ts)  // все время в миллисекундах
	{
		ScopedState state(GetState());  // удобный класс для контекстной блокировки 
		ExecutePipe(ts, true)  // сброс сигнала выставлен в true
	}
	
public:  
  	ActiveModule(int64_t int_param)
    	: int_param_(int_param)
    {
	}

	void StartImpl() override
	{	
		StartSomeThreadWithParam(int_param_);
	}

	void Stop() override
	{	
		StopThread();
	}
};
```

Python

```python
class ActiveModule(Module):

	def __init__(self):        
        super(ActiveModule, self).__init__()

	def StartImpl(self):
		startSomeThreadProc()

	def Stop(self):
		stopThreadProc()

	def callbackFromFuckingThread(self, ts: int):
		state = self.GetState()
		state.lock()
		self.ExecutePipe(ts, reset=true)
		state.unlock()
```


Активный модуль должен обязательно передать в пайп метку времени —  ее будут использовать все последующие модули. Это может быть текущее время, серверное время или прошлое, при работе с историческими данными.

Примеры запуска активного модуля:

С++

```c++
active_module.Start(&state, { &mod1, &mod2, &mod3 })
```

Python

```python
activeModule.Start(state, [mod1, mod2, mod3])
```

**ВАЖНО!** 
Оборачивайте, пожалуйста, тела методов в питоне в try except, иначе можно очень долго отлавливать неявный баг.

## Состояние

Состояние — это структура, в которой содержится вся информация, необходимая для общения между модулями:

* Список инструментов `Security`
* Текущий торговый сигнал `Signal`
* Текущие торговые ордера `Order`

Состояние создается один раз в одном экземпляре и используется на протяжении всей работы программы.
Состояние инициализируется списком предварительно созданных инструментов.

### Инструмент

Структура описывающая параметры и состояние торгуемого на какой-либо площадке инструмента. Состоит из нескольких секций. 
Первая секция содержит неизменяемые параметры, необходимые для идетификации инструмента в различных системах:

* `full_code` полный код контракта, например Si-12.16
* `class_code` идентификатор сессии, например CETS (валюта), SPBFUT (фьючерсы)
* `px_decimals` количество знаков после запятой в цене
* `px_step` шаг цены в пунктах
* `qty_decimals` количество знаков после запятой в объеме
* `qty_step` шаг объема в пунктах

Стакан заявок:

- `book_ts` время последнего изменения стакана (в мс); 0 означает, что стакан невалидный
- `bid_px` лучшая цена на покупку (в пунктах) 
- `bid_qty` объем по лучшей цене на покупку
- `ask_px` лучшая цена на продажу (в пунктах) 
- `ask_qty` объем по лучшей цене на продажу
- `min_px` минимальная цена в стакане (в пунктах) 
- `max_px` максимальная цена в стакане (в пунктах) 
- `book` массив объемов по указанной цене (индексы - цены в пунктах)

Последняя сделка:

- `trd_ts` время последней сделки (в мс)
- `trd_px` цена последней сделки (в пунктах) 
- `trd_qty`объем последней сделки

Позиция:

- `pos` позиция со знаком (+1 лонг, -1 шорт)
- `pos_buy_limit` суммарный объем висящих лимиток на покупку
- `pos_sell_limit` суммарный объем висящих лимиток на продажу
- `pos_buy_chunks` разбивка лонга по времени
- `pos_sell_chunks` разбивка шорта по времени

Настройки:

- `settings` строка с перечисленными потоками данных для данного иструмента
- `leg_factor` коэффициент ноги, имеет знак (1 общее направление, -1 обратное направление); значение 0 означает неторговый инструмент

Расширения:

- `extensions` словарь значений вещественного типа, где ключ - идентификатор, уникальный для каждого расширения стакана или сделок

#### Настройки 

Строка подписки на рыночные данные может содержать следующие значения (разделитель не важен, регистр нижний):

- `book1` подписка на лучшие цены
- `book5` подписка на стакан глубины 5
- `book20` подписка на стакан глубины 20
- `book50` подписка на стакан глубины 50
- `book` подписка на полный стакан
- `trades` подписка на сделки
- `finam` подписка на прогрев
- `zmq` подписка на котировки по 0mq

### Сигнал

Тоговый сигнал — структура, содержащая время, действие и направление торгового решения.
Временная метка сигналу присваивается активным модулем при запуске конвейера.
В сигнале может быть сколь угодно много торгуемых ног.

Напраление `Side` имеет для удобства представление в виде знака. Т.о. становятся удобными операции инвертирования направления и модификации позиции.

* 1 `Buy` 
* -1 `Sell` 
* 0 `All` 

Действия `Action` отсортированы в порядке наибольшего приоритета, это сделано для возможности фильтрации с помощью правил.

* 0 `Terminate` 
* 1 `ForceClose` 
* 2 `Bypass` 
* 3 `Close`
* 4 `Open`

### Ордер

Содержит информацию для торгового приказа: направление, количество, цену в десятичных пунктах, время постановки в миллисекундах и указатель на соответствующий инструмент.

Чтобы получить цену (объем) в виде числа с плавающей точкой, надо цену (объем) ордера поделить на 10 в степени количество знаков после запятой у данного инструмента: 

```python
order.px / 10 ** order.security.px_decimals
order.qty / 10 ** order.security.qty_decimals
```

Ордер может находиться в следующих состояниях:

* `Pending` ордер передан в обработку, но факт обработки не подтвержден (на примере zmq, ордер отсылается паблишером, до получения пакета со статусом `OrderStatus::Placed`)
* `Placed` ордер обработан (лимитка еще может быть не выставлена)
* `Filled` ордер частично или полностью исполнен
* `Canceled` ордер отвергнут системой или отменен, полностью или частично

Для удобства класс содержит методы, изменяющие состояние ордера:

* `Add`  обновляет метку времени и присваивает один из статусов `Pending` или `Placed`; позиция не изменяется, но модифицируются значения `pending_buy_limit` или `pending_sell_limit`. 
* `Fill`  вызывается при сделке, передаются время, объем и цена сделки; Происходит модификация позиции, текущих лимиток, а также разбивки позиции по времени `PositionChunk`. Это механизм для закрытия по времени FIFO. При каждой сделке "кусочки" позиции правильно схлопываются и объединяются так, что их сумма всегда равна текущей позиции.
* `Cancel` отмена ордер

### Кастомные поля в состоянии

В С++ нужно отнаследоваться от State, далее во всех местах кастовать указатели:

```c++
void Mutate(State* state) override
{
	ExtendedState* state_ex = dynamic_cast<ExtendedState*>(state);
}
```

В питоне поддерживаются динамические аттрибуты:

```python
def Mutate(self, state: State):
    print(state.some_new_var)
	state.another_extended_var = 42
```



### Предопределенные инструменты

Класс `SecurityStore` хранит список предопределенных инструментов. Создать экземпляр можно как по полному коду, так и по сокращению (удобно при перестановке контрактов перед экспирацией).
Используется метод `Create`, в который передается символьный ключ, а также настройки инструмента.
Класс также умеет правильно выбирать нужную экспирацию фьючерса на Московской бирже по символу, например Si.

```python
ss = SecurityStore()
si_sec = ss.Create(key='Si', settings="book,finam", leg_factor=1)
```

Пример сокращений и полных кодов предопределенных инструментов

|  Полный код  | Сокращение |
| :----------: | :--------: |
|   ED-12.16   |     ED     |
|   Si-12.16   |     Si     |
|   BR-1.17    |     BR     |
|  RTS-12.16   |    RTS     |
| USD000UTSTOM | USDRUB_TOM |




Библиотека модулей
==================

## Источник котировок `ModMcast`

Внутри представляет собой набор стримов, соответствующих потокам FAST, и набор расширений-обработчиков.
Стандартные расширения `ModMcastBook` (собирает стакан из обновлений) и `ModMcastTrades` (записывает последнюю сделку) активируются при наличии флагов `Flags::BookAggregate` и `Flags::Trades` соответственно.
Модуль самостоятельно выбирает стрим и обработчик для каждого инструмента состояния, на основе параметров `flags` и `session_id`.

Для правильной обработки котировок, следует понимать, в какой момент данные валидные.
Запуску конвейера предшествуют следующие события:

1. В FAST поток приходит новое сообщение
2. Декодировано первое обновление для одного из инструментов
3. Состояние блокируется
4. Применяются все последующие обновления
5. Очередь сообщений пуста
6. Запуск конвейера
7. Для каждого инструмента, если время последнего обновления (`trd_ts`, `book_ts`) не равно 0, то данные валидные 
8. Разблокировка состояния

В конце работы обязательно вызвать `Stop`.
Хорошей практикой будет запоминать в модуле стратегии время последнего обновления инструмента, чтобы пересчитывать только то, что изменилось.

### Расширения

Для каждого стрима на каждый инструмент создается обработчик, на который навешивается некоторое количество расширений. Расширения бывают встроенные (стакан, сделки), под их состояние заведены специальные переменные `book_*`  и `trd_*` и внешние, чьи значения хранятся в словаре `extensions`.

##### Низкоуровневые структуры

```c++
enum class FastUpdateAction
{
	New = 0,
	Change = 1,
	Delete = 2
};

enum class FastEntryType  // может быть расширена по запросу
{
	Bid = '0',
	Ask = '1',
	Trade = '2',
	Empty = 'J', 
};

struct FastBookUpdate
{
	FastUpdateAction action;
	FastEntryType type;
	int64_t ts;  // время в миллисекундах
	uint64_t level;  // для ФОРТС: уровень в стакане, отсчет с 1
	uint64_t rpt_seq;  // номер инкрементального обновления, должен увеличиваться на 1
	int64_t px;  // цена в пунктах
	int64_t qty;
};

class FastBook  // приведу только несколько методов, которые могут пригодиться
{
public:
  
	bool Online();  // стакан валидный
  	void RecoverFree();  // говорим, что не нужна процедура восстановления в случае пропуска
};
```



Расширение должно реализовывать интерфейс `ModMcastExtension`:

* `bool OnlineImpl(FastBook* book)` по умолчанию true, если хотя бы одно раширение возвратило false, пайп не исполнится
* `void ResetImpl(FastBook* book)` вызывается при начальной инициализации, при всех очистках стакана и клирингах
* `void UpdateImpl(FastBook* book, FastBookUpdate& update)` вызывается на каждое инкрементальное обновление, стакан еще не готов
* `void UpdateEndImpl(FastBook* book)` вызывается в конце тика, в этотм момент стакан уже собран, можно его использовать

```python
mcast = ModMcast([])
```

### Параметры конструктора

|    Название    |            Тип            | Описание                              |
| :------------: | :-----------------------: | :------------------------------------ |
| **extensions** | _List[ModMcastExtension]_ | список расширений (может быть пустой) |

Зависит от:

* `state.security.isin`
* `state.security.instr_id`
* `state.security.session_id`
* `state.security.book_depth`

Модифицирует:

* `state.signal.*`
* `state.security.book_*`
* `state.security.trd_*`

## Источник сделок `ModFinam`

Загружает сделки с выбранным таймфреймом и заданной глубиной (в днях). Выбирает инструменты по параметрам `flags` содержащим `FlagBoost`. Использует `finam_market` и `finam_quote`.
Следует вызывать в самом начале, блокирует состояние на весь период работы функции `Start`.

```python
finam = ModFinam(
    timeframe=FinamTimeFrame.Min1,
    day_shift=1,
    bar_open=True
)
```

### Параметры конструктора

|   Название    |       Тип        | Описание                                 |
| :-----------: | :--------------: | :--------------------------------------- |
| **timeframe** | _FinamTimeFrame_ | таймфрейм из перечисления `FinamTimeFrame` |
| **day_shift** |     _Int64_      | глубина истории в торговых днях          |
| **bar_open**  |      _Bool_      | если истина, то берется цена открытия бара, иначе - закрытия |

Зависит от:

* `state.security.flags`
* `state.security.finam_*`

Модифицирует:

* `state.signal.ts`
* `state.security.trd_*`

## Тайм менеджер `ModHours`

Хранит список торговых правил `ModHoursRule`, привязанных к определенному времени суток.
Каждое правило состоит из следующих полей:

* Время применения, строка `HH:MM:SS`
* Направление `Side`
* Действие `Action`

Если приоритет действия сигнала меньше текущего правила, то назначается действие соответствующее правилу.
Например, сигнал `Open`, а правило `ForceClose` для `All`, тогда итоговый сигнал будет `ForceClose`.

Модуль также имеет метод `ForceClose` для принудительного закрытия позиций. 
Параметр `min_period` задает минимальный перид в миллисекундах, в течение которого не будет применено следующее правило.

```python
hours = ModHours([
	ModHoursRule("11:00:00", Side.All, Action.Open),
	ModHoursRule("22:00:00", Side.All, Action.Close),        
	ModHoursRule("23:00:00", Side.All, Action.ForceClose)
])
```

### Параметры конструктора

| Название  |         Тип          | Описание     |
| :-------: | :------------------: | :----------- |
| **rules** | _List(ModHoursRule)_ | Набор правил |


Зависит от:

* `state.signal.ts`
* `state.signal.side`
* `state.signal.action`

Модифицирует:

* `state.signal.side`
* `state.signal.action`

## Генератор ордеров `ModOrderGen`

Преобразует сигнал в набор ордеров, в соответствии с коэффициентами ног. Если ордера в сигнале уже есть, то ничего не делает.
Для случаев `Close` и `ForceClose` с направлением `All` выбирает направление закрытия позиции по текущему инструменту.

```python
order_gen = ModOrderGen(lot_size=50)
```

### Параметры конструктора

|   Название   |   Тип   | Описание              |
| :----------: | :-----: | :-------------------- |
| **lot_size** | _Int64_ | Общий лот для ордеров |

Зависит от:

* `state.signal.action`
* `state.orders.empty`

Модифицирует:

* `state.orders`
* `state.signal.order.qty`
* `state.signal.order.side`
* `state.signal.order.security` (указатель)

## Менеджер позиций `ModPosLimit`

Ограничивает объем ордеров в соответствии с лимитами по позиции и открытым ордерам.

```python
pos_limit = ModPosLimit(max_pos=100)
```

### Параметры конструктора

|  Название   |   Тип   | Описание                    |
| :---------: | :-----: | :-------------------------- |
| **max_pos** | _Int64_ | Максимальный размер позиции |

Зависит от:

* `state.signal.action`
* `state.signal.order.side`
* `state.signal.order.qty`
* `state.signal.order.security.leg_factor`
* `state.signal.order.security.pos`
* `state.signal.order.security.pos_buy_limit`
* `state.signal.order.security.pos_sell_limit`

Модифицирует:

* `state.signal.order.qty`

## Исполнение по маркету `ModMarketPx`

Назначает цену для ордеров в сигнале. Также корректирует объем исходя из текущей ликвидности и минимально допустимого лота.

```python
market_px = ModMarketPx(min_qty=1, slippage=100)
```

### Параметры конструктора

|   Название   |   Тип   | Описание                   |
| :----------: | :-----: | :------------------------- |
| **min_qty**  | _Int64_ | Минимальный размер позиции |
| **slippage** | _Int64_ |                            |

Зависит от:

* `state.signal.order.side`
* `state.signal.order.qty`
* `state.signal.order.security.leg_factor`
* `state.signal.order.security.bid`
* `state.signal.order.security.ask`
* `state.signal.order.security.book`

Модифицирует:

* `state.signal.order.qty`
* `state.signal.order.px`

## Уравнитель `ModEqualizer`

Выравнивает объем в ордерах в соответствии с коэффициентом ног.

```python
equalizer = ModEqualizer()
```

Зависит от:

* `state.signal.order.qty`
* `state.signal.order.security.leg_factor`

Модифицирует:

* `state.signal.order.qty`

## Балансировщик `ModBalancer`

Используется только для синтетических инструментов. 
Обнаруживает разбалансировку синтетической позиции и генерирует соответствующие ордеры, с учетом предыдущего сигнала.
Работает только если нет неисполненных лимиток.

```python
balancer = ModBalancer()
```
Зависит от:

* `state.signal.action`
* `state.signal.order.side`
* `state.signal.order.qty`
* `state.signal.order.security.leg_factor`
* `state.signal.order.security.pos`
* `state.signal.order.security.pos_buy_limit`
* `state.signal.order.security.pos_sell_limit`

Модифицирует:

* `state.orders`
* `state.signal.order.qty`
* `state.signal.order.side`
* `state.signal.order.security` (указатель)

## Исполнение по сделкам `ModTrdPx`

Подходит для тестирования по сделкам. Назначает ордерам цену последней сделки плюс (минус) заданный в пунктах спред.

```python
trdPx = ModTrdPx(spread_pt=2)
```

### Параметры конструктора

|   Название    |   Тип   | Описание        |
| :-----------: | :-----: | :-------------- |
| **spread_pt** | _Int64_ | Спред в пунктах |

Зависит от:

* `state.signal.order.side`
* `state.signal.order.security.trd_px`

Модифицирует:

* `state.signal.order.px`

## Закрытие по времени `ModTimeClose`

Работает раз в минуту, суммирует "протухшие кусочки" позиции по каждому инструменту, и делает из них ордера, перезаписывая имеющиеся.
Делает сигнал `ForceClose`.

```python
time_close = ModVirtExec(timeout=1000*60)
```

### Параметры конструктора

|  Название   |   Тип   | Описание                 |
| :---------: | :-----: | :----------------------- |
| **timeout** | _Int64_ | время жизни позиции в мс |


Зависит от:

* `state.signal.ts`
* `state.signal.order.security.leg_factor`
* `state.signal.order.security.pos`
* `state.signal.order.security.pos_buy_chunks`
* `state.signal.order.security.pos_sell_chunks`

Модифицирует:

* `state.signal.action`
* `state.orders`
* `state.signal.order.qty`
* `state.signal.order.side`
* `state.signal.order.security` (указатель)

## Трейдер `ModVirtExec`

Виртуальная торговля, с рандомным исполнением (заявка может исполниться, а может повиснуть). Журналирует все сделки в файл.

```python
virt_exec = ModVirtExec(
    trade_timeout=1000, 
    cancel_timeout=3000, 
    trades_file='trades.csv', 
    random_unfill=False
)
```

### Параметры конструктора

|      Название      |   Тип    | Описание                                 |
| :----------------: | :------: | :--------------------------------------- |
| **trade_timeout**  | _Int64_  | отправление ордеров не чаще чем раз в промежуток (в мс) |
| **cancel_timeout** | _Int64_  | время до отмены ордера (в мс)            |
|  **trades_file**   | _String_ | путь к выходному файлу со сделками       |
| **random_unfill**  |  _Bool_  | случайно не исполняет ордера (имитирует висящие лимитки) |

Зависит от:

* `state.signal.ts`
* `state.signal.action`
* `state.signal.order.*`

Модифицирует:

* `state.signal.*`
* `state.signal.order.security.pos_*`

## Трейдер `ModPlazaExec`

Коннектор к плазе.

```python
plaza = ModPlazaExec(trade_timeout=1000, cancel_timeout=3000, comment='strategyName', client_code='CODE', port=1234)
```

### Параметры конструктора

|      Название      |   Тип    | Описание                                 |
| :----------------: | :------: | :--------------------------------------- |
| **trade_timeout**  | _Int64_  | отправление ордеров не чаще чем раз в промежуток (в мс) |
| **cancel_timeout** | _Int64_  | время до отмены ордера (в мс)            |
|    **comment**     | _String_ | Комментарий к выставляемым ордерам       |
|  **client_code**   | _String_ | зависит от счета                         |
|      **port**      | _Short_  | зависит от инстанса роутера, по умолчанию 4001 |

Зависит от:

* `state.signal.ts`
* `state.signal.action`
* `state.signal.order.*`

Модифицирует:

* `state.signal.*`
* `state.signal.order.security.pos_*`


## Точка восстановления позиции `ModRestorePoint`

Сохраняет текущую позицию при отсутствии висящих лимиток и загружает при старте приложения.

```python
restore = ModRestorePoint('restore_point.csv')
```

### Параметры конструктора

| Название |   Тип    | Описание                             |
| :------: | :------: | :----------------------------------- |
| **path** | _String_ | путь к файлу с точкой восстановления |

Зависит от:

- `state.security.class_code`
- `state.security.full_code`
- `state.security.pos_buy_limit`
- `state.security.pos_sell_limit`
- `state.security.leg_factor`

Модифицирует:

- `state.security.pos`

## ZeroMQ-based модули

Все взаимодействие построено по схеме PUB-SUB через посредника. Решается проблема обнаружения, а также временного отключения одной из сторон. Адреса посредника на M1:

* `tcp://127.0.0.1:42001` для отправки сообщений
* `tcp://127.0.0.1:42002` для получения сообщений

### Получение сигналов `ModZmqSignalSub`

```python
sig_sub = ModZmqSignalSub(name='signal_name', forwarder_rx="tcp://127.0.0.1:42002")
```

#### Параметры конструктора

|     Название     |   Тип    | Описание                                 |
| :--------------: | :------: | :--------------------------------------- |
|     **name**     | _String_ | уникальное имя сигнала (используется как топик сообщения) |
| **forwarder_rx** | *String* | zmq адрес получения                      |

Модифицирует:

- `state.signal`

### Отправка сигналов `ModZmqSignalPub`

```python
sig_pub = ModZmqSignalPub(name='signal_name', forwarder_tx="tcp://127.0.0.1:42001")
```

#### Параметры конструктора

|     Название     |   Тип    | Описание                                 |
| :--------------: | :------: | :--------------------------------------- |
|     **name**     | _String_ | уникальное имя сигнала (используется как топик сообщения) |
| **forwarder_tx** | *String* | zmq адрес отправки                       |

Зависит от:

- `state.signal`

### Получение котировок `ModZmqSecuritySub`

Подписывается на инструменты с флагом `zmq`, топик `SEC_<ClassCode>_<FullCode>`. Запускает конвеер на каждый тик.

```python
sec_sub = ModZmqSecuritySub(forwarder_rx="tcp://127.0.0.1:42002")
sec_sub.Start(state, [])
sec_sub.Stop()
```

#### Параметры конструктора

|     Название     |   Тип    | Описание            |
| :--------------: | :------: | :------------------ |
| **forwarder_rx** | *String* | zmq адрес получения |

Зависит от:

- `state.securities.settings`
- `state.securities.class_code`
- `state.securities.full_code`

Модифицирует:

* `state.securities.book_*`
* `state.securities.trd_*`
* `state.signal.ts`

### Отправка котировок `ModZmqSecurityPub`

Для всех инструментов с флагом `zmq` рассылаются обновления с  топиком `SEC_<ClassCode>_<FullCode>`.

```python
sec_sub = ModZmqSecurityPub(forwarder_tx="tcp://127.0.0.1:42001")
```

#### Параметры конструктора

|     Название     |   Тип    | Описание           |
| :--------------: | :------: | :----------------- |
| **forwarder_tx** | *String* | zmq адрес отправки |

Зависит от:

- `state.securities.settings`
- `state.securities.class_code`
- `state.securities.full_code`


- `state.securities.book_*`
- `state.securities.trd_*`

### Исполнение `ModZmqExec`

Реализует торговлю по алгоритму, аналогичному `ModVirtExec`: постановка лимиток, отмена через таймаут. Дополнительный параметр `placed_timeout` введен для обработки случая, когда принимающая сторона недоступна, если через заданный промежуток времени не пришло подтверждение со статусом `OrderStatus::Placed`, считается, что ордер отвергнут.

Благодаря дизайну PUB-SUB можно исполнять ордера сразу на нескольких удаленный экзекьюторах, например реализовывать арбитражные стратегии.

```python
executor = ModZmqExec(
    comment="test",
    place_timeout=5000,
    cancel_timeout=3000,
    forwarder_rx="tcp://127.0.0.1:42002",
	forwarder_tx="tcp://127.0.0.1:42001"
)
executor.Start(state, [])
executor.Stop()
```

#### Параметры конструктора

|      Название      |    Тип    | Описание                             |
| :----------------: | :-------: | :----------------------------------- |
|    **comment**     | *String*  | комментарий к ордерам                |
| **place_timeout**  | *Integer* | таймаут ответа о постановке ордера   |
| **cancel_timeout** | *Integer* | таймаут отмены неисполненных лимиток |
|  **forwarder_rx**  | *String*  | zmq адрес получателя                 |
|  **forwarder_tx**  | *String*  | zmq адрес отправителя                |

Зависит от:

- `state.orders`
- `state.signal.ts`
- `state.securities.full_code`


- Модифицирует:
  - `state.signal.*`
  - `state.signal.order.security.pos_*`

## Инкрементальные индикаторы

Базовый класс `RollingBase` для имплеметации индикаторов со скользящим окном. 
Доступные для переопределения методы: `OnPush`, `OnPop`, `calculate`.

Стандартные индикаторы (параметры: `window`, `timeframe`):

* `RollingMean`
* `RollingStd`
* `RollingMin`
* `RollingMax`
* `RollingShift`
* `RSI`

Использование:

1. Проинициализировать (в конструкторе).
2. Обновлять внутреннее значение на каждой итерации с помощью `Update` (принимает `ts` время в мс и `value` вещественное число).
3. Когда индикатор полностью "прогреется", метод `Good` будет возвращать истину.
4. Получить конечное значение можно с помощью `calculate`. (У некоторых индикаторов есть вспомогательные методы, например `mean` у `RollingStd`, или `previous` у `RollingMinMax`).

```python
	oil_rsi = RSI(window, timeframe)
	oil_sec = state.securities[1]
	oil_rsi.Update(state.signal.ts, oil_sec.trd_px)
	if oil_rsi.Good():
		rsi = oil_rsi.calculate()
		print('RSI: %.f', rsi)
```




Установка
==================

### Откуда качать

* cgate ftp://ftp.moex.com/pub/FORTS/Plaza2/CGate/ (x64)
* libzmq https://github.com/zeromq/libzmq
* conda https://www.continuum.io/downloads (x64)
* pybind11 https://github.com/pybind/pybind11.git

### Прописать переменные окружения

* `%CGATE_HOME%` (C:\Moscow Exchange\SpectraCGate2)
* `%CPP_DEPS%` (C:\deps)
* `%CONDA_HOME%` (C:\Anaconda3)

### Проверить зависимости в `%PATH%` (приложения будут искать здесь библиотеки)

* `%CGATE_HOME%\bin`
* `%CONDA_HOME%`
* `%CONDA_HOME%\Library\bin`
