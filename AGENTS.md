Hi Mr Agent!

# Documentation

Here are your entrypoints for anything you may want to know about this project:

* README.md (You probably want to just preload this)
* docs/dev_guide/index.md
* docs/user_guide.md

The public headers also contain a lot of information.

# Rules to follow in this town

If you have to be in here, then abide by the following:

* There are no compromises to be made when it comes down to lvkw_event_queue_push()
* Dont use defensive programming against anything covered by API_VALIDATION
* Wrap all string buffers destined for Diagnostics in LVKW_ENABLE_DIAGNOSTICS ifdefs
* Don't run wayland-scanner, use the generator in /scripts