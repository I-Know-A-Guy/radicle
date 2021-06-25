# Radicle

A C library containing common functionalities like database access or authentication using cookies to help grow REST API's. 

## Functionalities

Radicle offers: 
 - High level [libpq](https://www.postgresql.org/docs/current/libpq.html) bindings
 - Authentication and sessions using cookies

## [Documentation](https://radicle-docs.netlify.app/index.html)
[![Netlify Status](https://api.netlify.com/api/v1/badges/f702492b-49ca-4d82-9e3e-5dbd9690a0e4/deploy-status)](https://app.netlify.com/sites/radicle-docs/deploys)

## Installing

### Dependencies

- [Argon2](https://github.com/P-H-C/phc-winner-argon2) is for password hashing.
- [PostgreSQL](https://www.postgresql.org/) is used for data persistence.
- [OpenSSL](https://www.openssl.org/) is used for random data generation and base64 encoding/decoding.
- [Yasm](http://yasm.tortall.net/) is used by Subhook to fake functions for unit testing.

### Already integrated 

- [Subhook](https://github.com/Zeex/subhook) is used for faking functions.
- [GoogleTest](https://github.com/google/googletest) used when testing code. This results in the testing code being C++. Will be downloaded by CMake.

