
FighterStats loadStats()
{
	FighterStats stats;
	stats.modelFilename = "humanmodelNoBones.osg";
	stats.setResistance(HOT, 0.0);
	stats.setResistance(COLD, 0.0);
	stats.setResistance(ACID, 0.0);
	stats.maxHealth = 25.0;
	stats.weaponType = "Staff";
	stats.setWeaponStats(loadWeaponPrototype(stats.weaponType));
	stats.weaponStats.setProjectileStats(loadProjectilePrototype("Fireball"));
	return stats;
}
