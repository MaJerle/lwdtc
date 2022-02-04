.. _cron_format:

CRON format
===========

LwDTC is a lightweight cron library and supports very basic cron string parsing. Supported characters are:

- Numbers between min and maximum field value
- ``-`` separator for range definition
- ``,`` for multi value specification
- ``/`` for step definition

.. note::
	Comparing to standard linux CRON where fixed date in month and week day are bitwise-ORed, meaning cron will fire 
    on day in month match or on week day match, LwDTC will only fire if both parameters are match at the same time.

    In practice, setting cron to fire on ``month-day = 15`` and ``week-day = 6``, will trigger it only on 15th in a month which is also Saturday at the same time. 

Have a look at :ref:`examples` for several example use cases

.. toctree::
    :maxdepth: 2
