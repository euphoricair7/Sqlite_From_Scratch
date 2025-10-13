#!/usr/bin/env bash

set -e

echo "🔧 Setting up Ruby dependencies..."
if ! command -v bundle &> /dev/null; then
    echo "Installing bundler..."
    gem install bundler
fi

bundle install

echo "🏗️  Compiling C program..."
gcc -Wall -Wextra -std=c11 -o maincode maincode.c

echo "🧪 Running tests..."
bundle exec rspec --format documentation --color

echo "✅ All tests completed!"