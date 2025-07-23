EXAMPLE = cylinder3d
OLB_ROOT := ../../..
include $(OLB_ROOT)/default.mk

# Direactorios
DAT = data
FIG = fig
TMP = tmp

run_simul: $(EXAMPLE)
	@echo
	@read -p "  >> Ingrese el número de procesos para ejecutar el programa: " N; \
	echo; \
	echo "  Ejecutando $(EXAMPLE) con $$N procesos"; \
	mpirun --use-hwthread-cpus -np $$N ./$(EXAMPLE); \
	echo "Programa ejecutado con $$N procesos."

metrics: $(EXAMPLE)
	bash run.sh
	@echo "Generando gráficas de métricas en carpeta $(FIG)"
	gnuplot metrics.gp

clean_simul:
	@echo "Cleaning /$(DAT)"
	rm -rf $(DAT)
	@echo "Cleaning /$(FIG)"
	rm -rf $(FIG)
	@echo "Cleaning /$(TMP)"
	rm -rf $(TMP)