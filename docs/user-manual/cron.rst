.. _cron_format:

CRON
====

LwDTC provides ultra-lightweight support for simple CRON implementation.
This page tends to provide quick information to be able to quickly move forward.

In the current revision library doesn't support true scheduler,
instead it supports CRON string parser, and dedicated function to check if CRON is valid against compared time.

.. tip::
    User can implement its own CRON loop, that reads current time and checks if parsed CRON is valid for specific time.
    Use :cpp:func:`lwdtc_cron_is_valid_for_time` to compare

Supported characters
********************

- Numbers between minimum and maximum value for each of date and time field
- Support for ``seconds``, ``minutes``, ``hours``, ``day-in-month``, ``month``, ``day-in-week`` and ``year``,
- ``-`` is used to define value range, with min and max boundaries, ``min-max`` or ``max-min``
- ``,`` is used to specify multiple fixed values
- ``/`` is used to define step between min and max values
- ``*`` is used to represent *any* value

.. note::
    Comparing to standard linux CRON, where fixed date in month and week day are bitwise-ORed, meaning cron will fire
    on day in month match or on week day match, LwDTC does it more simple. Date&Time is valid only if both parameters are a match at the same time.
    In practice, setting cron to fire on ``month-day = 15`` and ``week-day = 6``, will trigger it only on **15th** in a month which is also **Saturday** at the same time. 

CRON string format
******************

To define valid CRON string, a string with ``7`` parameters, separated by space, must be provided:

``seconds minutes hours day-in-month month day-in-week year``

Each of them has to be present to consider CRON as valid input.

CRON examples
*************

This section provides list of some CRON examples in its default configuration.

+-------------------------+----------------------------------------------------------------------------------+
| CRON string             | Description                                                                      |
+=========================+==================================================================================+
| ``* * * * * * *``       | CRON is valid all the time, will fire every second                               |
+-------------------------+----------------------------------------------------------------------------------+
| ``0 * * * * * *``       | CRON is valid at the beginning of each minute                                    |
+-------------------------+----------------------------------------------------------------------------------+
| ``* * * * * 2 *``       | CRON is valid every Tuesday all day long                                         |
+-------------------------+----------------------------------------------------------------------------------+
| ``0 0 13-15 * * 2-4 *`` | CRON is valid every beginning of the minute between hours 13-15 afternoon,       |
|                         | between Tuesday and Thursday                                                     |
+-------------------------+----------------------------------------------------------------------------------+
| ``*/5 * * * * * *``     | CRON is valid every ``5`` seconds starting at ``0``                              |
+-------------------------+----------------------------------------------------------------------------------+
| ``*/5 */5 * * * * *``   | CRON is valid every ``5`` seconds each ``5`` minutes, from ``00:00`` to ``55:55``|
+-------------------------+----------------------------------------------------------------------------------+
| ``0 0 0 * * 5 *``       | Every Friday at midnight                                                         |
+-------------------------+----------------------------------------------------------------------------------+
| ``0 0 */2 * * * *``     | Every ``2`` hours at beginning of the hour                                       |
+-------------------------+----------------------------------------------------------------------------------+
| ``* * */2 * * * *``     | Every second of every minute every ``2`` hours (``0, 2, 4, .., 22``)             |
+-------------------------+----------------------------------------------------------------------------------+
| ``0 0 0 * * 1-5 *``     | At midnight, ``00:00`` every week between Monday and Friday                      |
+-------------------------+----------------------------------------------------------------------------------+
| ``15 23 */6 * * * *``   | Every ``6`` hours at (min:sec) ``23:15`` (``00:23:15, 06:23:15, 12:23:15``, ...) |
+-------------------------+----------------------------------------------------------------------------------+
| ``0 0 0 1 * * *``       | At ``00:00:00`` beginning of the month                                           |
+-------------------------+----------------------------------------------------------------------------------+
| ``0 0 0 1 */3 * *``     | Every beginning of the quarter at 00:00:00                                       |
+-------------------------+----------------------------------------------------------------------------------+
| ``10 15 20 * 8 6 *``    | At ``20:15:20`` every Saturday in August                                         |
+-------------------------+----------------------------------------------------------------------------------+
| ``10 15 20 8 * 6 *``    | At ``20:15:20`` every Saturday that is also ``8th`` day in month                 |
|                         | (both must match, day Saturday and date ``8th``)                                 |
+-------------------------+----------------------------------------------------------------------------------+
| ``30-45 * * * * * *``   | Every second between ``30`` and ``45``                                           |
+-------------------------+----------------------------------------------------------------------------------+
| ``30-45/3 * * * * * *`` | Every ``3rd`` second in every minute, when seconds are between ``30`` and ``45`` |
+-------------------------+----------------------------------------------------------------------------------+
| ``0 23/1 * * * * *``    | Every beginning of a minute when minute is between ``23`` and ``59``             |
+-------------------------+----------------------------------------------------------------------------------+
| ``50-10 * * * * * *``   | Every second when seconds are from ``50-59`` and ``00-10`` (overflow mode)       |
+-------------------------+----------------------------------------------------------------------------------+

.. literalinclude:: ../../examples/cron_basic.c
    :language: c
    :linenos:
    :caption: Basic CRON example with parser

.. toctree::
    :maxdepth: 2
