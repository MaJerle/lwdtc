.. _cron_basic_schedule:

CRON basic schedule
===================

The idea behind cron is to schedule tasks at specific interval or period of time.

Library does not provide generic scheduler that would do that automatically for you,
instead user should manually implement custom scheduler to periodically check and schedule should cron execute or not.

.. tip::
    A check should be performed at least at minimum cron granularity, that being ``1 second`` in the current revision.

Basic example is very simple and does the following:

* Parses cron defined by user with :cpp:`lwdtc_cron_parse` function
* Reads system time periodically (example was tested under *Windows* environment)
* It checks if cron is valid for execution each time new time changes versus previous check
* It executes task

.. literalinclude:: ../../examples/cron_basic.c
    :language: c
    :linenos:
    :caption: Basic CRON example with parser
