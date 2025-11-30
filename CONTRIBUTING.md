# Contributing to OpenModbus

Thank you for your interest in contributing to OpenModbus! This document provides guidelines and instructions for contributing to this project.

## Code of Conduct

By participating in this project, you are expected to uphold our Code of Conduct:
- Be respectful and inclusive
- Exercise consideration and respect in your speech and actions
- Attempt collaboration before conflict
- Refrain from demeaning, discriminatory, or harassing behavior

## How Can I Contribute?

### Reporting Bugs

Before creating a bug report, please check the existing issues to avoid duplicates.

**How to Submit a Good Bug Report:**
- Use a clear and descriptive title
- Describe the exact steps to reproduce the problem
- Provide specific examples to demonstrate the steps
- Describe the behavior you observed and what you expected
- Include any relevant logs, screenshots, or code examples
- Specify your environment (OS, compiler, hardware)

### Suggesting Enhancements

We welcome suggestions for new features and improvements!

**How to Submit a Good Enhancement Suggestion:**
- Use a clear and descriptive title
- Provide a detailed description of the proposed functionality
- Explain why this enhancement would be useful
- Include examples of how the feature would be used
- List any alternative solutions you've considered

### Pull Requests

1. **Fork the Repository**
   ```bash
   git clone --recurse-submodules https://github.com/sszczep/OpenModbus.git
   cd OpenModbus
   ```

2. Create a Feature Branch
    ```bash
    git checkout -b feature/amazing-feature
    # or
    git checkout -b fix/issue-description
    ```

3. Make Your Changes
    - Follow the coding standards below

    - Add tests for new functionality

    - Update documentation as needed

4. Run Tests
    ```bash
    make
    ```

5. Commit Your Changes
    ```bash
    git commit -s -m "Add amazing feature"
    ```

6. Push to Your Fork
    ```bash
    git push origin feature/amazing-feature
    ```

7. Open a Pull Request

    - Use a clear title and description

    - Reference any related issues

    - Ensure all checks pass

## Coding Standards

### C Code Style

- Follow the existing code style

- 4-space indentation (no tabs)

- 80-character line limit where practical

- Use descriptive variable and function names

### Example Code Style

```c
/**
 * Brief description of function
 * 
 * @param param1 Description of first parameter
 * @param param2 Description of second parameter
 * @return Description of return value
 */
static int example_function(int param1, const char *param2) {
    if (param1 == 0) return -1;
    
    // Complex operations should have comments
    int result = perform_operation(param1, param2);
    
    return result;
}
```

## Documentation

- Document all public APIs with Doxygen-style comments

- Update README.md for user-facing changes

- Comment complex algorithms and business logic

- Keep comments up-to-date with code changes

## Testing Requirements

- All new features must include unit tests

- Bug fixes should include regression tests

- Maintain 100% test coverage for new code

- Tests should be independent and repeatable

## Commit Guidelines

Commit Message Format:

```text
<type>: <description>

[optional body]

[optional footer]
```

Types:

- `feat`: New feature

- `fix`: Bug fix

- `docs`: Documentation changes

- `style`: Code style changes (formatting, etc.)

- `refactor`: Code refactoring

- `test`: Adding or updating tests

- `chore`: Maintenance tasks

## Questions?

- Open an issue for technical questions

- Join our discussions for design questions

- Contact maintainers for sensitive issues

Thank you for contributing to OpenModbus! 🚀