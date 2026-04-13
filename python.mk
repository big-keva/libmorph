# Makefile для сборки Python wheels
# Использует образ manylinux

IMAGE := manylinux
VERSION := 4.1.0
PLATFORM := manylinux2014_x86_64

# Директории
PYRUSMORPH_DIR := pyrusmorph
PYENGMORPH_DIR := pyengmorph

# Версии setuptools
SETUPTOOLS_OLD := "setuptools==65.5.0"   # для Python 3.7-3.11
SETUPTOOLS_NEW := "setuptools==70.3.0"   # для Python 3.12

# Все поддерживаемые версии Python
PYTHON_VERSIONS := cp37-cp37m cp38-cp38 cp39-cp39 cp310-cp310 cp311-cp311 cp312-cp312

# Цвета для вывода
GREEN := \033[0;32m
RED := \033[0;31m
NC := \033[0m

.PHONY: help clean clean-all pyrusmorph-ext pyengmorph-ext all-ext

help:
	@echo "Доступные цели:"
	@echo "  make pyrusmorph-ext  - собрать wheels для pyrusmorph"
	@echo "  make pyengmorph-ext  - собрать wheels для pyengmorph"
	@echo "  make all-ext         - собрать оба пакета"
	@echo "  make clean           - очистить временные файлы"
	@echo "  make clean-all       - полная очистка (включая dist)"
	@echo ""
	@echo "Версия пакета: $(VERSION)"
	@echo "Платформа: $(PLATFORM)"

clean:
	@echo "$(GREEN)Очистка временных файлов...$(NC)"
	@find $(PYRUSMORPH_DIR) -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	@find $(PYRUSMORPH_DIR) -type d -name "*.egg-info" -exec rm -rf {} + 2>/dev/null || true
	@find $(PYRUSMORPH_DIR) -type d -name "build" -exec rm -rf {} + 2>/dev/null || true
	@find $(PYENGMORPH_DIR) -type d -name "__pycache__" -exec rm -rf {} + 2>/dev/null || true
	@find $(PYENGMORPH_DIR) -type d -name "*.egg-info" -exec rm -rf {} + 2>/dev/null || true
	@find $(PYENGMORPH_DIR) -type d -name "build" -exec rm -rf {} + 2>/dev/null || true
	@echo "$(GREEN)Готово$(NC)"

clean-all: clean
	@echo "$(GREEN)Полная очистка dist...$(NC)"
	@rm -rf $(PYRUSMORPH_DIR)/dist $(PYENGMORPH_DIR)/dist
	@echo "$(GREEN)Готово$(NC)"

pyrusmorph-ext:
	@echo "$(GREEN)=== Сборка pyrusmorph $(VERSION) ===$(NC)"
	@mkdir -p $(PYRUSMORPH_DIR)/dist
	@for pyver in $(PYTHON_VERSIONS); do \
		echo "$(GREEN)Сборка для $$pyver...$(NC)"; \
		if [ "$$pyver" = "cp312-cp312" ]; then \
			SETUPTOOLS_VER=$(SETUPTOOLS_NEW); \
		else \
			SETUPTOOLS_VER=$(SETUPTOOLS_OLD); \
		fi; \
		docker run --rm \
			--user root \
			-v $(PWD):/project \
			-w /project/$(PYRUSMORPH_DIR) \
			$(IMAGE) \
			bash -c "\
				echo 'Установка pip, '$$SETUPTOOLS_VER', wheel, pybind11...' && \
				/opt/python/$$pyver/bin/python -m pip install --quiet --upgrade pip $$SETUPTOOLS_VER wheel pybind11 && \
				echo 'Сборка wheel...' && \
				/opt/python/$$pyver/bin/python setup.py bdist_wheel --plat-name=$(PLATFORM) && \
				chmod 644 /project/$(PYRUSMORPH_DIR)/dist/*.whl && \
				echo 'Готово для $$pyver'"; \
		if [ $$? -eq 0 ]; then \
			echo "$(GREEN)✓ $$pyver готов$(NC)"; \
		else \
			echo "$(RED)✗ $$pyver не удался$(NC)"; \
		fi; \
	done
	@echo "$(GREEN)=== Готово: $(PYRUSMORPH_DIR)/dist/ ===$(NC)"
	@ls -la $(PYRUSMORPH_DIR)/dist/

pyengmorph-ext:
	@echo "$(GREEN)=== Сборка pyengmorph $(VERSION) ===$(NC)"
	@mkdir -p $(PYENGMORPH_DIR)/dist
	@for pyver in $(PYTHON_VERSIONS); do \
		echo "$(GREEN)Сборка для $$pyver...$(NC)"; \
		docker run --rm \
			--user root \
			-v $(PWD):/project \
			-w /project/$(PYENGMORPH_DIR) \
			$(IMAGE) \
			bash -c "\
				echo 'Установка pip, setuptools, wheel, pybind11...' && \
				/opt/python/$$pyver/bin/python -m pip install --quiet --upgrade pip setuptools wheel pybind11 && \
				echo 'Сборка wheel...' && \
				/opt/python/$$pyver/bin/python setup.py bdist_wheel --plat-name=$(PLATFORM) && \
				chmod 644 /project/$(PYENGMORPH_DIR)/dist/*.whl && \
				echo 'Готово для $$pyver'"; \
		if [ $$? -eq 0 ]; then \
			echo "$(GREEN)✓ $$pyver готов$(NC)"; \
		else \
			echo "$(RED)✗ $$pyver не удался$(NC)"; \
		fi; \
	done
	@echo "$(GREEN)=== Готово: $(PYENGMORPH_DIR)/dist/ ===$(NC)"
	@ls -la $(PYENGMORPH_DIR)/dist/

all-ext: pyrusmorph-ext pyengmorph-ext
	@echo "$(GREEN)=== Все пакеты собраны ===$(NC)"
