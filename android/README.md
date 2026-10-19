# Development signer

`dev-test.jks` is a deliberately public, non-secret test-only signer. Both passwords
are `android`; alias `flower-test`. Keep this key stable for compatible test updates.
Never use it for production. No prior Flower package or signer existed in this line.
The NativeActivity packaging route uses Crystal's successful AGP 8.7.3 / Gradle 8.9
evidence, with Flower's own identity and library. It does not assert phone acceptance.
