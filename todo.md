## TODO

- refactor of scheduler to struct doesn't work with tcb creation and overall thread actions (like scheduler_yield) as the client threads don't have access to scheduler object.
instead they could reference the global scheduler by default
- write unit tests for scheduler

### chat notes on my TODO

- refactor of scheduler to struct doesn't work with tcb creation and overall thread actions (like scheduler_yield) as the client threads don't have access to scheduler object.
instead they could reference the global scheduler by default
- write unit tests for scheduler

