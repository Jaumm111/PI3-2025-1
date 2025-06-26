# Terceira Etapa

## Estrutura física da seletora

Utilizou-se como material uma combinação de MDF, MDP e madeira natural para criar a estrutura composta por:

- 2 portas móveis atuadas por motores de passo e sua estrutura de suporte.
- 1 suporte tipo mesa para fixação da câmera e restante da parte eletrônica.
- 1 cancela para reter o avanço das frutas sub a câmera, possibilitanto girar a fruta.

Todo o material foi doado ou reaproveitado. Encontrou-se inicialmente dificuldades devido à inexperiência com trabalho em madeira, porém significativas evoluções ocorreram. A primeira mesa montada pela equipe, vista na imagem abaixo, ficou bastante torta e pouco estável. Esta mesa foi refeita, com outro material e utilizando-se de outras técnicas de fixação e de design, gerando um resultado muito mais bem acabado e estável.

![imagemmesavelha](../images/entrega3/mesa.jpg)

A nova mesa pode ser vista na imagem abaixo, além de uma estrutura mais reforçada, o método de fixação deixou de ser pregos e passou a ser parafusos transpassados e porcas.

![mesanova](../images/entrega3/mesanova.jpg)

Foram também feitas as portas que serão responsáveis por impedir que as maçãs caiam nos compartimentos errados. Essas portas podem ser vistas na imagem abaixo.

![portas](../images/entrega3/2portas.jpg)

## Treinamento e implementação de modelo

### Treinamento inicial

Utilizando-se do google colab treinou-se um modelo YOLOv 5S com a base de dados selecionada na etapa 2, em testes utilizando imagens de alta resolução e sem pré-processamento algum o modelo apresentou uma alta taxa de falsos positivos, o modelo não foi testado com muitas imagens ou com pré-processamento. O modelo levou 11 ms para classificar uma imagem de alta resolução em um computador desktop.

Um breve snippet do trinamento pode ser visto abaixo

```


```

### Implementação

Por não apresentar resultados satisfatórios este modelo não foi implementado no ESP, optando-se por implementar um modelo pré treinado para verificar a viabiliade do ESP escolhido para a classificação.

Implementou-se então um modelo pré treinado que classifica parafusos em 3 categorias: pequeno, grande e preto. O modelo foi pré treinado com 300 epochs, sendo apenas um modelo exemplo, ele foi capaz de distinguir parafusos de outros objetos, e apresentou performance satisfatória para os testes iniciais em classificar por tamanho. O modelo pré treinado levou 1,4 segundos para classificar uma imagem, indicando que ou otimizações serão necessárias, ou será necessário o uso de hardware mais potente para o produto final.

Uma breve demonstração pode ser vista abaixo.

![gifparafuso](../images/entrega3/briangif.gif)


## Teste dos acionamentos


![video](../images/entrega3/acionamento_cancela.mp4)


## Sinconização dos atuadores



## Referências

[^1]: Borba, K. R., Sapelli, K. S., Spricigo, P. C., & Ferreira, M. D. (2017). Near infrared spectroscopy sugar quantification in intact orange. Citrus Research & Technology, 38(2). https://doi.org/10.4322/crt.ICC118

[^2]: Bahareh Jamshidi, Ability of near-infrared spectroscopy for non-destructive detection of internal insect infestation in fruits: Meta-analysis of spectral ranges and optical measurement modes, Spectrochimica Acta Part A: Molecular and Biomolecular Spectroscopy, Volume 225, 2020, 117479,ISSN 1386-1425, https://doi.org/10.1016/j.saa.2019.117479.

[^3]: Boulley, L et al. 2015. Near-infrared optical spectroscopy of sugar-based mixtures – A snapshot to identify issues of influence. In: Längle, T et al (eds.), OCM 2015 – 2nd International Conference on Optical Characterization of Materials, March 18th – 19th, 2015, Karlsruhe, Germany : Conference Proceedings. Karlsruhe: KIT Scientific Publishing. https://doi.org/10.58895/ksp/1000044906-3 

[^4]: Ji, W.; Wang, J.; Xu, B.; Zhang, T. Apple Grading Based on Multi-Dimensional View Processing and Deep Learning. Foods 2023, 12, 2117. https://doi.org/10.3390/foods12112117

[^5]: Naik, Sapan & Patel, Bankim. (2017). Machine Vision based Fruit Classification and Grading - A Review. International Journal of Computer Applications. 170. 22-34. 10.5120/ijca2017914937. 

