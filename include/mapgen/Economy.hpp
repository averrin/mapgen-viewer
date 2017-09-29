#ifndef ECONOMY_H_
#define ECONOMY_H_

namespace Economy {
  const float POPULATION_GROWS = 0.04;
  const float POPULATION_GROWS_WEALTH_MODIFIER = 2;

  const float PACKAGES_PER_NICE = 15;
  const float PACKAGES_AGRO_POPULATION_MODIFIER = 1;

  const float PACKAGES_PER_MINERALS = 10;
  const float PACKAGES_MINERALS_POPULATION_MODIFIER = 1;

  const float CONSUME_AGRO_POPULATION_MODIFIER = 0.5;
  const float CONSUME_MINERALS_POPULATION_MODIFIER = 0.5;

  const float CONSUME_AGRO_WEALTH_MODIFIER = 0.05;
  const float CONSUME_MINERALS_WEALTH_MODIFIER = 0.05;

  const float CANT_BUY_AGRO = 0.01;
  const float CANT_BUY_MINERALS = 0.01;

  const float PORT_FEE = 0.3;
}

#endif
