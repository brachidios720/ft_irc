/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hehuang <hehuang@student.42lehavre.fr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/23 22:16:00 by hehuang           #+#    #+#             */
/*   Updated: 2025/02/23 22:58:01 by hehuang          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Includes/Server.hpp"

int main (int argc, char *argv[]) {
	if (argc == 3)
	{
		Server a = Server(argv[1], argv[2]);
		a.run();
	}
	return 0;
}
