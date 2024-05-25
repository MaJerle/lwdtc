.. _cron:

CRON date&time range
====================

Motivation for LwDTC library comes with my home automation project, where I need a simple way to define range of date or time, from and to, in very flexible and field-agnostic way.
It should be possible to just define from and to seconds within a minute, or from ``2`` different minutes and different seconds.

CRON concept doesn't provide such functionality. To overcome such problem, a simple solution with multiple cron objects can be implemented.
Consider a task, that needs to execute each beginning of a minute, between ``Monday starting at 07:00:00`` and ``Friday ending at 19:30:00``.

For CRON-like compatible syntax, defined range needs a split to:

- *Monday*: CRON is active every beginning of a minute from ``07`` to ``23`` hours. This can be described as: ``0 * 7/1 * * 1 *``
- *Tue,Wed,Thu*: CRON is active at beginning of each minute for all ``3`` days: ``0 * * * * 2-4 *``
- *Friday*:

  - CRON is active between ``0`` and ``19`` hours, at beginning of each minute: ``0 * 7-19 * * 5 *``
  - CRON is also active at beginning of each minute, when minutes are between ``0`` and ``30`` and when hour is ``19``: ``0 0-30 19 * * 5 *``

This gives us in total ``4`` different cron objects, for which:

- We need to parse all of them
- To check if particular time is within range, an *OR* operation between all ranges is performed

.. literalinclude:: ../../examples/cron_dt_range.c
    :language: c
    :linenos:
    :caption: CRON date&time range descriptor

.. toctree::
    :maxdepth: 2
