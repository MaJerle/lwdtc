.. _cron_multi_schedule:

CRON multi schedule
===================

Consider a task that has to execute at different times or periods of time, for example:

* *Each Friday at exactly midnight* ``0 0 0 * * 5 *`` and
* *Each second every Tuesday* ``* * * * * 2 *``

With CRON syntax, it is not possible to describe this with one string, as it does not allow multi range option.
Solution is to rather use *multiple* definitions and then let scheduler to check all of them until at least one is a match.

.. tip::
    LwDTC comes with ``*_multi`` functions allowing you to check several CRON contextes with single function call.

An example shows simple demo how to implement a task scheduler which executes at different CRON context-es.
It implements calls to ``_multi`` functions for simplification

.. literalinclude:: ../../examples/cron_multi.c
    :language: c
    :linenos:
    :caption: CRON execution at multiple ranges
